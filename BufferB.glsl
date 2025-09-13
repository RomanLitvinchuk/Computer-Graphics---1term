void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float cellSize = 50.0;

    vec2 cellIndex = floor(fragCoord / cellSize);

    int x = int(cellIndex.x);
    int y = int(cellIndex.y);

    bool isBlack = ((x + y) % 2) == 0;

    fragColor = isBlack ? vec4(0.0, 0.0, 0.0, 1.0) : vec4(1.0, 1.0, 1.0, 1.0);
}