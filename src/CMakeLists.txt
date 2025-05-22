add_executable(SocialForce
"main.cpp"
"std_image.cpp"
"Control.cpp"
# "Goal.cpp"
"Pedestrian.cpp"
# "Spawn.cpp"
"Wall.cpp"
"Graphics.cpp"
) #list all cpp files
target_link_libraries(SocialForce
glfw
glm::glm
glad
imgui
)