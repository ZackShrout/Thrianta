#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(set = 0, binding = 0) uniform global_uniform_object
{
    mat4 projection;
	mat4 view;
} globalUBO;

void main()
{
    gl_Position = globalUBO.projection * globalUBO.view * vec4(in_position, 1.0);
}