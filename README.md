# Grafický Engine Demo

Demo projekt(y), který používá můj [Grafický Engine](https://github.com/bagons/graphicengine)

## Dema

**DEMO0** - Unit testy

**DEMO1** - Crytek Sponza + First Person Controller
- *Ovládání:* **WSAD**, **SPACE** = Skok, **SHIFT** = Běh, **F11** = fullscreen, **ESC** = uvolnění myší, **LEVÉ TLAČÍTKO MYŠI** = znovu zamknout myš
- *PS*: chvíli může trvat, než se velký model načte

## Instalace

Stačí zkompilovat projekt pomocí CMakeLists.txt souboru.

```bash
git clone --recurse-submodules https://github.com/bagons/ge-demos.git
cd ge-demos
cmake .
cmake --build .
```

## Požadavky

CMake verze 3.31

Může se stát, že nebudete mít všechny požadavky na vytvoření okna, jsou to [požadavky knihovny GLFW](https://www.glfw.org/docs/latest/compat.html), kterou engine používá. Naštěstí vás na to upozorní a po případné instalaci nedostatků vše funguje.