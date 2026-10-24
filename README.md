# Bizatune (VST3 para FL Studio)

Autotune estilo trap underground. Controles: Tono, Escala, Velocidad de corrección (-100 a 100) y Mezcla.
La velocidad en 0 salta a la nota al instante; los valores negativos robotizan más (pasos más duros, resonancia metálica y toque digital).

## Compilar en Windows
1. Instalá Visual Studio 2022 (o Build Tools) con "Desarrollo para el escritorio con C++", y CMake.
2. En una terminal, dentro de esta carpeta:

    cmake -B build -G "Visual Studio 17 2022"
    cmake --build build --config Release

   La primera vez descarga JUCE solo (hace falta internet).
3. Copiá la carpeta `build/Bizatune_artefacts/Release/VST3/Bizatune.vst3` a `C:\Program Files\Common Files\VST3`.
   También se genera una versión Standalone para probar sin FL Studio.

## Usar en FL Studio
1. Options > Manage plugins > "Find plugins" (Start scan).
2. Abrí Bizatune como efecto en una pista del Mixer, con la voz sola en esa pista.
3. FL compensa la latencia (~12 ms) automáticamente.

## Compilar sin instalar nada (GitHub)
1. Creá un repositorio en GitHub y subí el contenido de esta carpeta (incluida `.github`).
2. Entrá a la pestaña "Actions", abrí "Compilar Bizatune" y esperá a que termine (unos minutos).
3. Descargá el resultado "Bizatune-VST3". Adentro hay:
   - `Bizatune-un-solo-archivo.vst3`: un único archivo. Renombralo a `Bizatune.vst3` y copialo a `C:\Program Files\Common Files\VST3`.
   - `Bizatune.vst3`: la versión en carpeta (formato estándar de VST3), por si la primera no se detecta.
