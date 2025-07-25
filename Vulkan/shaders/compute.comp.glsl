// glslc -O -fshader-stage=compute -x glsl .\compute.comp.glsl -o compute.comp.spv

#version 450

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in; // these are defaults

layout(binding = 0) buffer Input {
	uint values[];
};

layout (constant_id = 0) const uint NUM_BUFFER_ELEMENTS = 0; // needs a default despite it being changable by the CPU

void main(){
	uint index = gl_GlobalInvocationID.x;

	if(index >= NUM_BUFFER_ELEMENTS){ return; }

	values[index] = values[index] * 2;
	// values[index] = NUM_BUFFER_ELEMENTS;
}

