#version 410 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 vertex_color;

void main()
{
    vertex_color = in_color;
    gl_Position = u_projection * u_view * u_model * vec4( in_position, 1.0 );
}
