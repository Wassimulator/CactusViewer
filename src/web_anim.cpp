
#include <webp/demux/anim_decode.c>
struct Decoded_Frame{
	uint8_t* rgba;         // Decoded and reconstructed full frame.
	int duration;          // Frame duration in milliseconds.
};

struct Animated_Image {
	uint32_t canvas_width;
	uint32_t canvas_height;
	uint32_t bgcolor;
	uint32_t loop_count;
	Decoded_Frame* frames;
	uint32_t num_frames;
	void* raw_mem;
	int* durations;
};


static int webp_anim_allocate_frames(Animated_Image* const image, uint32_t num_frames) {
	uint32_t i;
	static const int kNumChannels = 4;
	const size_t rgba_size =
	image->canvas_width * kNumChannels * image->canvas_height;
	uint8_t* const mem = (uint8_t*)malloc(num_frames * rgba_size * sizeof(*mem));
	Decoded_Frame* const frames =
	(Decoded_Frame*)malloc(num_frames * sizeof(*frames));
	int *durations = (int*)malloc(num_frames * sizeof(int));

	if (mem == NULL || frames == NULL) {
		free(mem);
		free(frames);
		return 0;
	}
	free(image->raw_mem);
	image->num_frames = num_frames;
	image->frames = frames;
	for (i = 0; i < num_frames; ++i) {
		frames[i].rgba = mem + i * rgba_size;
		frames[i].duration = 0;
	}
	image->raw_mem = mem;
	image->durations = durations;
	return 1;
}

// Read animated WebP bitstream 'filename' into 'AnimatedImage' struct.
static int webp_anim_read_file(const WebPData* const webp_data, Animated_Image* const image) {
	static const int kNumChannels = 4;
	int ok = 0;
	int dump_ok = 1;
	uint32_t frame_index = 0;
	int prev_frame_timestamp = 0;
	WebPAnimDecoder* dec;
	WebPAnimInfo anim_info;

	memset(image, 0, sizeof(*image));

	dec = WebPAnimDecoderNew(webp_data, NULL);
	if (dec == NULL) {
		fprintf(stderr, "Error parsing image");
		goto End;
	}

	if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
		fprintf(stderr, "Error getting global info about the animation\n");
		goto End;
	}

	// Animation properties.
	image->canvas_width = anim_info.canvas_width;
	image->canvas_height = anim_info.canvas_height;
	image->loop_count = anim_info.loop_count;
	image->bgcolor = anim_info.bgcolor;

	// Allocate frames.
	if (!webp_anim_allocate_frames(image, anim_info.frame_count)) return 0;

	// Decode frames.
	while (WebPAnimDecoderHasMoreFrames(dec)) {
		Decoded_Frame* curr_frame;
		uint8_t* curr_rgba;
		uint8_t* frame_rgba;
		int timestamp;

		if (!WebPAnimDecoderGetNext(dec, &frame_rgba, &timestamp)) {
			fprintf(stderr, "Error decoding frame #%u\n", frame_index);
			goto End;
		}
		assert(frame_index < anim_info.frame_count);
		curr_frame = &image->frames[frame_index];
		curr_rgba = curr_frame->rgba;
		curr_frame->duration = timestamp - prev_frame_timestamp;
		image->durations[frame_index] = curr_frame->duration;
		memcpy(curr_rgba, frame_rgba,
		       image->canvas_width * kNumChannels * image->canvas_height);
		++frame_index;
		prev_frame_timestamp = timestamp;
	}
	ok = 1;

	End:
		WebPAnimDecoderDelete(dec);
	return ok;
}
