#include <fenv.h>
#include <thread>
#include "mandelbrot.hxx"
#include "shade.hxx"
#include "pngWriter.hxx"
#include "argsParser.hxx"
#include "socket.hxx"
#include "ringBuffer.hxx"
#include "conversions.hxx"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
using std::operator ""s;
#pragma GCC diagnostic pop

const static arg_t args[] =
{
	{"--nodes", 1, 1, 0},
	{"--self", 1, 1, 0},
	{"--compute", 1, 1, 0},
	{"-w", 1, 1, 0},
	{"-h", 1, 1, 0},
	{"-s", 1, 1, 0},
	{nullptr, 0, 0, 0}
};
parsedArgs_t parsedArgs;

constexpr double zoom = 1;
constexpr static const point2_t center{-0.5, 0};
const char *self = nullptr;
std::vector<std::string> nodes;
uint32_t width = 0, height = 0, subdiv = 0, compNodes = 0, selfIndex = 0;
uint32_t xTiles = 0, yTiles = 1;
point2_t region;
bool multiProcess;
std::vector<uint32_t> availableProcessors;

void writeImage(std::unique_lock<std::mutex> &lock) noexcept
{
	for (uint32_t i{0}; i < height; ++i)
	{
		while (imageStatus[i] < xTiles)
			imageSync.wait_for(lock, 1s);
		writePNGRow(i, width);
		fflush(stdout);
	}
}

int server(socketStream_t &socket) noexcept
{
	const area_t size{width, height};
	std::unique_lock<std::mutex> lock(imageMutex);
	auto shaderThreads = makeUnique<std::thread []>(compNodes - 1);
	image = makeUnique<rgb8_t []>(width * height);
	imageStatus = makeUnique<std::atomic<uint32_t> []>(height);
	if (!image || !imageStatus || !openPNG(size))
		return 1;
	printf("Setting up render of %u by %u Mandelbrot Set\n", width, height);
	for (uint32_t i{0}; i < height; ++i)
		imageStatus[i] = 0;

	if (!socket.listen(nodes[0].data(), 2000))
	{
		puts("Failed to listen on port 2000");
		return 2;
	}
	puts("Shader process ready for connetions");

	const area_t subchunk{width / xTiles, height / yTiles};
	printf("Launching shaders for %u subchunks of %u by %u\n",
		compNodes - 1, subchunk.width(), subchunk.height());
	for (uint32_t i{1}; i < compNodes; ++i)
	{
		const uint32_t tile = i - 1;
		const area_t location{tile % xTiles, tile / xTiles};
		socketStream_t stream = socket.accept();
		if (!stream.valid())
		{
			puts("Failed to aquire socket for incomming connection");
			abort();
		}

		shaderThreads[tile] = std::thread([=](socketStream_t stream, const uint32_t affinityOffset) noexcept
			{ shadeChunk(size, subchunk, subdiv * subdiv, stream, affinityOffset); },
			std::move(stream), i
		);
	}

	fflush(stdout);
	writeImage(lock);
	puts("Reaping shaders");
	for (uint32_t i{1}; i < compNodes; ++i)
		shaderThreads[i - 1].join();
	closePNG();
	return 0;
}

int client(stream_t &stream) noexcept
{
	const area_t size{width, height};
	const area_t subchunk{size / area_t{xTiles, yTiles}};
	const point2_t scale{size / region};
	const uint32_t tile = selfIndex ? selfIndex - 1 : 0;
	const area_t location{tile % xTiles, tile / xTiles};

	if (multiProcess)
	{
		puts("Compute process sleeping 1 second before starting");
		std::this_thread::sleep_for(1s);
		socketStream_t &socket = reinterpret_cast<socketStream_t &>(stream);
		if (!socket.connect(nodes[0].data(), 2000))
		{
			puts("Failed to connect to the shader process");
			return 2;
		}
	}

	printf("Computing a subchunk of %u by %u, at location %u, %u\n",
		subchunk.width(), subchunk.height(), location.width(), location.height());
	write(stream, location);
	computeChunk(subchunk, subchunk * location, scale, center, subdiv, stream);
	return 0;
}

bool imageSize() noexcept
{
	const toInt_t<uint32_t> widthStr(findArg(parsedArgs, "-w", nullptr)->params[0].get());
	const toInt_t<uint32_t> heightStr(findArg(parsedArgs, "-h", nullptr)->params[0].get());
	const toInt_t<uint32_t> subdivStr(findArg(parsedArgs, "-s", nullptr)->params[0].get());
	const toInt_t<uint32_t> xTilesStr(findArg(parsedArgs, "--compute", nullptr)->params[0].get());
	if (!widthStr.isInt() || !heightStr.isInt() || !subdivStr.isInt() ||
		!xTilesStr.isInt() || xTilesStr > compNodes || subdivStr == 0)
		return false;
	const uint32_t computeNodes = compNodes - 1;
	if (multiProcess && (computeNodes % xTilesStr) != 0)
		return false;
	std::tie(width, height, subdiv, xTiles) = std::tie(widthStr, heightStr, subdivStr, xTilesStr);
	return true;
}

void calculateRegion() noexcept
{
	double base = std::min(width, height) / (2.0 / zoom);
	region = {width / base, height / base};
	if (multiProcess)
		yTiles = (compNodes - 1) / xTiles;
}

void masterAffinity() noexcept
{
	cpu_set_t affinity = {};
	sched_getaffinity(0, sizeof(cpu_set_t), &affinity);
	printf("Allocated to %u CPUs\n", CPU_COUNT(&affinity));

	try
	{
		for (uint32_t i{0}; i < CPU_SETSIZE; ++i)
		{
			if (CPU_ISSET(i, &affinity))
				availableProcessors.push_back(i);
		}
	}
	catch (const std::bad_alloc &) { abort(); }
	if (!availableProcessors.size())
		abort();

	threadAffinity(0);
}

int main(int argc, char **argv) noexcept
{
	registerArgs(args);
	parsedArgs = parseArguments(argc, argv);
	if (!parsedArgs || countParsedArgs(parsedArgs) != 6)
	{
		puts("Failed to parse my command line arguments");
		return 2;
	}

	self = findArg(parsedArgs, "--self", nullptr)->params[0].get();
	try
		{ nodes = parseNodelist(findArg(parsedArgs, "--nodes", nullptr)->params[0].get()); }
	catch (const std::bad_alloc &) { abort(); }
	compNodes = nodes.size();
	multiProcess = compNodes > 1;

	for (uint32_t i{0}; i < compNodes; ++i)
	{
		if (strcmp(nodes[i].data(), self) == 0)
		{
			selfIndex = i;
			break;
		}
	}

	if (!imageSize())
	{
		puts("Width and height and subdivisions must all be positive integral values");
		puts("and the number of divisions of x specified must be cleanly divisible into the number of workers");
		return 1;
	}
	calculateRegion();

	fenv_t fenv;
	if (feholdexcept(&fenv))
	{
		puts("Unable to save the floating point environment");
		return 1;
	}
	masterAffinity();

	if (multiProcess)
	{
		socketStream_t stream{socketType_t::ipv4};
		if (nodes[0] == self)
			return server(stream);
		return client(stream);
	}
	else
	{
		ringStream_t stream;
		std::unique_lock<std::mutex> lock(imageMutex);

		image = makeUnique<rgb8_t []>(width * height);
		imageStatus = makeUnique<std::atomic<uint32_t> []>(height);
		if (!image || !imageStatus || !openPNG({width, height}))
			return 1;
		for (uint32_t i{0}; i < height; ++i)
			imageStatus[i] = 0;

		std::thread computeThread(client, std::ref(stream));
		std::thread shaderThread([](stream_t &stream) noexcept
			{ shadeChunk({width, height}, {width, height}, subdiv * subdiv, stream, (subdiv * subdiv) + 1); },
			std::ref(stream)
		);

		writeImage(lock);
		computeThread.join();
		shaderThread.join();
		closePNG();
	}

	feupdateenv(&fenv);
	return 0;
}
