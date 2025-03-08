#version 330 core
in float Height; // Odbieramy wysokość z vertex shadera

out vec4 FragColor; // Kolor wyjściowy

void main()
{
    // Normalizacja wysokości do zakresu [0, 1]
    float normalizedHeight = (Height + 10.0) / 20.0; // Zakładamy, że wysokość jest w zakresie [-10, 10]

    // Interpolacja między ciemnym a jasnym kolorem w zależności od wysokości
    vec3 darkColor = vec3(0.0, 0.0, 0.5); // Ciemny kolor (np. niebieski)
    vec3 lightColor = vec3(0.8, 0.8, 1.0); // Jasny kolor (np. jasnoniebieski)

    // Mieszanie kolorów na podstawie wysokości
    vec3 finalColor = mix(darkColor, lightColor, normalizedHeight);

    FragColor = vec4(finalColor, 1.0); // Ustawienie koloru wyjściowego
}