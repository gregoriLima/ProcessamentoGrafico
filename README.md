# ProcessamentoGrafico

Projeto independente da tarefa 4, pronto para compilar fora da pasta original.

## Estrutura

- `src/tarefa_parte4.cpp`: codigo-fonte principal.
- `assets/`: texturas usadas pela aplicacao.
- `common/glad.c`: loader OpenGL.
- `include/`: headers locais de GLAD, GLM e `stb_image.h`.
- `third_party/glfw/`: codigo-fonte local do GLFW para build offline.

## Como buildar

No PowerShell, entre na pasta `build` e execute:

```powershell
cmake --build .
```

## Como executar

```powershell
.\tarefa_parte4.exe
```

O executavel deve ser iniciado a partir da pasta `build` para que as texturas em `..\assets\` sejam encontradas corretamente.