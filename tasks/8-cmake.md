Look, cmake is not cool on vs code.

It needs some modifications to work properly.

On boot i got this debug log which implies that every build project is build into the root of the repository build folder, when it should be compartimentalized in the project folder.

# Log
```plaintext
[bookmarks] Indicadores 0 carregados
[proc] Executando o comando: /usr/bin/cmake --version
[proc] Executando o comando: /usr/bin/cmake -E capabilities
[kit] Êxito ao carregar 2 kits de /home/jvcm/.local/share/CMakeTools/cmake-tools-kits.json
[variant] Novo conjunto de variantes carregado
[variant] Novo conjunto de variantes carregado
[proc] Executando o comando: /usr/bin/gcc -v
[proc] O comando: ninja --version falhou com erro: Error: spawn ninja ENOENT
[proc] O comando: ninja-build --version falhou com erro: Error: spawn ninja-build ENOENT
[variant] Novo conjunto de variantes carregado
[variant] Novo conjunto de variantes carregado
[proc] Executando o comando: /usr/bin/cmake -S /home/jvcm/notas/10_Projects/PSI3441/entregas/5-1/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/build -G "Unix Makefiles"
[proc] O comando: /usr/bin/cmake -S /home/jvcm/notas/10_Projects/PSI3441/entregas/5-1/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/build -G "Unix Makefiles" saiu com o código: 1
[variant] Novo conjunto de variantes carregado
[main] Configurando projeto: 3 
[main] Configurando projeto: PSI3441 
[proc] Executando o comando: /usr/bin/cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ --no-warn-unused-cli -S /home/jvcm/notas/10_Projects/PSI3441/entregas/5-1/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/build -G "Unix Makefiles"
[proc] Executando o comando: /usr/bin/cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S /home/jvcm/notas/10_Projects/PSI3441/entregas/template/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/entregas/3/build
[cmake] CMake Error at CMakeLists.txt:2 (include):
[cmake]   include could not find requested file:
[cmake] 
[cmake]     /home/jvcm/.local/share/platformio/packages/framework-zephyr/cmake/app/boilerplate.cmake
[cmake] 
[cmake] 
[cmake] Not searching for unused variables given on the command line.
[cmake] CMake Error at CMakeLists.txt:9 (target_sources):
[cmake]   Cannot specify sources for target "app" which is not built by this project.
[cmake] 
[cmake] 
[cmake] -- Configuring incomplete, errors occurred!
[proc] O comando: /usr/bin/cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ --no-warn-unused-cli -S /home/jvcm/notas/10_Projects/PSI3441/entregas/5-1/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/build -G "Unix Makefiles" saiu com o código: 1
[cmake] CMake Error at CMakeLists.txt:2 (include):
[cmake]   include could not find requested file:
[cmake] 
[cmake]     /home/jvcm/.local/share/platformio/packages/framework-zephyr/cmake/app/boilerplate.cmake
[cmake] 
[cmake] 
[cmake] Not searching for unused variables given on the command line.
[cmake] -- Configuring incomplete, errors occurred!
[cmake] CMake Error at CMakeLists.txt:9 (target_sources):
[cmake]   Cannot specify sources for target "app" which is not built by this project.
[cmake] 
[cmake] 
[proc] O comando: /usr/bin/cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S /home/jvcm/notas/10_Projects/PSI3441/entregas/template/zephyr -B /home/jvcm/notas/10_Projects/PSI3441/entregas/3/build saiu com o código: 1
[proc] Executando o comando: /usr/bin/arm-none-eabi-gcc -v
[proc] O comando: ninja --version falhou com erro: Error: spawn ninja ENOENT
[proc] O comando: ninja-build --version falhou com erro: Error: spawn ninja-build ENOENT
[main] Configurando projeto: 5 
```