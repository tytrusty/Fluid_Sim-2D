Ty Trusty

Current implementation includes basic fluid simulation based off of Stam's original model for fluid flow. I am currently extending this model to support liquid simulation. I am doing this with a level set method that keeps track of the fluid's surface and then I advect this signed distance along the velocity field. Currently there are issues with rendering the fluid and preserving the volume of the liquid. 

My next steps are to fix the immediate issues with the rendering and volume and then 

## Build and Run
In the main directory:
- mkdir build; cd build
- cmake ..
- ./bin/fluid

## Features/Usage:
- Add "dye" (density) into the simulation and move it along the vector field. Right click the screen to add density the simulation. The fluid simulation advects and diffuses this dye to create a smoke-like appearance. 

- Display velocity field. Press keyboard key 'v' to display the velocity field.

- Display the super duper basic heat boundary. Press 'h' to display this. As the boundary expands, the viscosity of the cells over which it expands will lower. This will be used to simulate the melting.

- Apply External forces. Left click and drag the screen to do this. You can see the forces applied by pressing 'v' to show the velocity field. 

- Apply Gravitational force. Press 'g' to do this. 

- Decrease/Increase time step with <- and -> arrow keys.

- Decrease/Increase viscosity with \[ and \] keys. This is great for testing the heat boundary. 

- Renders the liquid surface using the marching cubes algorithm. (Currently what I'm working on)
