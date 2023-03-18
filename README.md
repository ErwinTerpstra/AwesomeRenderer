# Awesome renderer

## About the project

This project has been a sandbox for many different techniques I've wanted to experiment with. The largest part is a physically based path tracer, it it also contains a basic OpenGL renderer and a pure software renderer.

## From scratch

One of my (foolish) goals with this project was to write all the supporting code from the ground up. The project contains routines to load meshes, textures, normal maps and do Window management with the Win32 API.

While the learning experience of this route is interesting, it does make progress a bit slow.

## Path tracer

The path tracer part of this project has by far the most work put into it. It has many features such as a physically based BRDF, explicit light sampling, multiple importance sampling and spatial acceleration trough a K-D tree.

It also supports most material properties you would expect from a PBR renderer, such as texture maps for albedo, normals, specular and roughness. Mip-mapping for all these textures is supported as well.

## Disclaimer

The code in this project is ridiciously slow and over-engineered. Only use it as a reference.
