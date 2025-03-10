#version 330 core
in float Height; // Odbieramy wysokość z vertex shadera

out vec4 FragColor; // Kolor wyjściowy

void main()
{
    // Normalizacja wysokości do zakresu [0, 1]
    float normalizedHeight = (Height + 10.0) / 20.0; // Zakładamy, że wysokość jest w zakresie [-10, 10]

    // Interpolacja między ciemnym a jasnym kolorem w zależności od wysokości
    vec3 darkColor = vec3(0.4, 0.2, 0.1); // Ciemny kolor (np. ciemny szary)
    vec3 lightColor = vec3(0.7, 0.6, 0.5); // Jasny kolor (np. jasnoszary)

    // Mieszanie kolorów na podstawie wysokości
    vec3 finalColor = mix(darkColor, lightColor, normalizedHeight);

    FragColor = vec4(finalColor, 1.0); // Ustawienie koloru wyjściowego
}