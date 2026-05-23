#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/* ===== CONSTANTES ===== */

const int WIDTH = 800;                      // largura da tela em pixels
const int HEIGHT = 600;                     // altura da tela em pixels

const float PASSO_PERSONAGEM = 12.0f;       // quantidade de movimento por tecla
const float TAMANHO_PERSONAGEM = 96.0f;     // tamanho do personagem na tela
const float POSICAO_INICIAL_PERSONAGEM_Y = HEIGHT * 0.68f; // posicao vertical inicial do personagem
const float LIMITE_ESQUERDO_X = WIDTH * 0.30f; // faixa esquerda onde o personagem pode andar
const float LIMITE_DIREITO_X = WIDTH * 0.70f; // faixa direita onde o personagem pode andar
const float VELOCIDADE_CAMERA_DENTRO_FAIXA = 0.35f; // mundo anda pouco enquanto o personagem ainda pode se mover
const float VELOCIDADE_CAMERA_NO_LIMITE = 1.20f; // mundo anda mais quando o personagem encosta no limite

const float CHAO_CENTRO_Y = 545.0f;         // centro Y da camada de chao (deve bater com camadaChao)
const float CHAO_ALTURA = 150.0f;           // altura da camada de chao (deve bater com camadaChao)
const float TOPO_DO_CHAO_Y = CHAO_CENTRO_Y - CHAO_ALTURA / 2.0f - TAMANHO_PERSONAGEM / 2.0f; // centro maximo do personagem sobre o chao
const float LIMITE_INFERIOR_POR_PASSOS = POSICAO_INICIAL_PERSONAGEM_Y + 4.0f * PASSO_PERSONAGEM; // limite pedido de 4 movimentos para baixo
const float LIMITE_INFERIOR_Y = TOPO_DO_CHAO_Y < LIMITE_INFERIOR_POR_PASSOS ? TOPO_DO_CHAO_Y : LIMITE_INFERIOR_POR_PASSOS; // usa o menor limite entre passos e chao

/* ===== ESTRUTURAS SIMPLES ===== */

struct Camada {                             // guarda os dados de uma camada do fundo
    GLuint textura;                         // textura usada pela camada
    float largura;                          // largura desenhada da camada
    float altura;                           // altura desenhada da camada
    float centroY;                          // posicao vertical do centro da camada
    float velocidade;                       // quanto a camada acompanha o personagem
};

/* ===== ESTADO DO JOGO ===== */

float personagemX = WIDTH * 0.50f;          // posicao horizontal do personagem
float personagemY = POSICAO_INICIAL_PERSONAGEM_Y; // posicao vertical do personagem
float cameraX = 0.0f;                       // deslocamento acumulado horizontal do mundo

/* ===== FUNCOES AUXILIARES ===== */

float limitar(float valor, float minimo, float maximo) {      // mantem um valor dentro do intervalo
    if (valor < minimo) {                                     // verifica limite minimo
        return minimo;                                        // devolve o minimo
    }

    if (valor > maximo) {                                     // verifica limite maximo
        return maximo;                                        // devolve o maximo
    }

    return valor;                                             // devolve valor original
}

int checarShader(GLuint shader, const char* nome) {           // confere se o shader compilou
    GLint compilado = 0;                                      // recebe o estado de compilacao
    char infoLog[512];                                        // recebe a mensagem de erro

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilado);     // consulta o OpenGL
    if (!compilado) {                                         // se falhou
        glGetShaderInfoLog(shader, 512, NULL, infoLog);       // busca o texto do erro
        fprintf(stderr, "ERRO DE COMPILACAO NO SHADER %s\n%s\n", nome, infoLog);
        return 0;                                             // indica falha
    }

    return 1;                                                 // indica sucesso
}

int checarPrograma(GLuint programa, const char* nome) {       // confere se o programa linkou
    GLint linkado = 0;                                        // recebe o estado do link
    char infoLog[512];                                        // recebe a mensagem de erro

    glGetProgramiv(programa, GL_LINK_STATUS, &linkado);       // consulta o OpenGL
    if (!linkado) {                                           // se falhou
        glGetProgramInfoLog(programa, 512, NULL, infoLog);    // busca o texto do erro
        fprintf(stderr, "ERRO DE LINK NO PROGRAMA %s\n%s\n", nome, infoLog);
        return 0;                                             // indica falha
    }

    return 1;                                                 // indica sucesso
}

int carregarTexturaArquivo(const char* file_name, GLuint* tex) {  // carrega uma imagem PNG em uma textura
    int x, y, n;                                                  // largura, altura e canais da imagem
    int force_channels = 4;                                       // força RGBA para facilitar transparencia
    unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);

    if (!image_data) {                                            // se nao carregou
        return 0;                                                 // indica erro
    }

    glGenTextures(1, tex);                                        // cria a textura
    glActiveTexture(GL_TEXTURE0);                                 // usa a unidade 0
    glBindTexture(GL_TEXTURE_2D, *tex);                           // ativa a textura criada
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);                              // gera mipmaps

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    stbi_image_free(image_data);                                  // libera a imagem da memoria
    return 1;                                                     // indica sucesso
}

void obterDiretorioFonte(char* diretorio, int tamanho) {          // descobre a pasta deste arquivo cpp
    int i;                                                        // contador para percorrer a string

    snprintf(diretorio, tamanho, "%s", __FILE__);               // copia o caminho do arquivo fonte

    for (i = (int)strlen(diretorio) - 1; i >= 0; i--) {           // procura a ultima barra
        if (diretorio[i] == '\\' || diretorio[i] == '/') {       // achou separador de pasta
            diretorio[i + 1] = '\0';                             // corta depois da barra
            return;                                               // termina a funcao
        }
    }

    diretorio[0] = '\0';                                         // usa pasta vazia se nao achou
}

int carregarTexturaRelativaAoFonte(const char* caminhoRelativo, GLuint* tex) {  // carrega textura pelo caminho relativo
    char diretorioFonte[1024];                                                   // pasta do arquivo cpp
    char caminhoCompleto[1024];                                                  // caminho final da textura

    obterDiretorioFonte(diretorioFonte, sizeof(diretorioFonte));                 // pega a pasta src
    snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s%s", diretorioFonte, caminhoRelativo);

    if (carregarTexturaArquivo(caminhoCompleto, tex)) {                          // tenta carregar a imagem
        printf("TEXTURA CARREGADA: %s\n", caminhoCompleto);                    // mostra sucesso
        return 1;                                                                // indica sucesso
    }

    fprintf(stderr, "ERRO: NAO FOI POSSIVEL CARREGAR %s\n", caminhoCompleto);  // mostra erro
    return 0;                                                                    // indica falha
}

void desenharQuad(GLuint programa, GLuint VAO, GLuint textura,
                  float x, float y, float largura, float altura,
                  int flipVertical) {                                             // 1 = inverte o sprite verticalmente
    glm::mat4 model = glm::mat4(1.0f);                                           // matriz inicial sem transformacao
    model = glm::translate(model, glm::vec3(x, y, 0.0f));                        // move o quad para a posicao
    model = glm::scale(model, glm::vec3(largura, altura, 1.0f));                 // aumenta para o tamanho desejado

    glUniformMatrix4fv(glGetUniformLocation(programa, "model"),                // envia matriz model
                       1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(programa, "basic_texture"), 0);           // informa a textura 0
    glUniform1i(glGetUniformLocation(programa, "flip_vertical"), flipVertical); // envia flag de inversao

    glBindVertexArray(VAO);                                                      // ativa o VAO do quadrado
    glActiveTexture(GL_TEXTURE0);                                                // usa unidade 0
    glBindTexture(GL_TEXTURE_2D, textura);                                       // escolhe a textura
    glDrawArrays(GL_TRIANGLES, 0, 6);                                            // desenha dois triangulos
    glBindVertexArray(0);                                                        // desativa o VAO
}

void desenharCamada(GLuint programa, GLuint VAO, Camada camada) {               // desenha uma camada com repeticao lateral continua
    float deslocamentoX = -cameraX * camada.velocidade;                         // parallax horizontal
    float xBase = WIDTH * 0.50f + deslocamentoX;                                // posicao inicial de referencia
    float yBase = camada.centroY;                                                // altura fixa da camada

    // recua xBase ate a borda esquerda do tile estar antes de x=0
    while (xBase - camada.largura * 0.5f > 0.0f) {
        xBase -= camada.largura;
    }

    // avanca caso o tile esteja completamente fora pela esquerda
    while (xBase + camada.largura * 0.5f < 0.0f) {
        xBase += camada.largura;
    }

    // desenha tiles da esquerda para a direita ate cobrir toda a tela
    float x = xBase;
    while (x - camada.largura * 0.5f < (float)WIDTH) {                          // enquanto o tile ainda tocar a tela
        desenharQuad(programa, VAO, camada.textura, x, yBase, camada.largura, camada.altura, 1);
        x += camada.largura;                                                    // avanca para o proximo tile
    }
}

void moverPersonagem(float dx, float dy) {                         // altera personagem e camera juntos
    personagemY = limitar(personagemY + dy, 80.0f, LIMITE_INFERIOR_Y); // move so o personagem no eixo Y sem mexer o mundo

    if (dx > 0.0f) {                                               // trata movimento para direita
        if (personagemX < LIMITE_DIREITO_X) {                      // enquanto ainda estiver dentro da faixa jogavel
            personagemX = limitar(personagemX + dx, LIMITE_ESQUERDO_X, LIMITE_DIREITO_X); // move o personagem ate o limite
            cameraX = cameraX + dx * VELOCIDADE_CAMERA_DENTRO_FAIXA; // fundo anda pouco enquanto ele caminha
        } else {                                                   // quando ja encostou no limite direito
            cameraX = cameraX + dx * VELOCIDADE_CAMERA_NO_LIMITE;  // fundo anda mais rapido e personagem fica parado
        }
    }

    if (dx < 0.0f) {                                               // trata movimento para esquerda
        if (personagemX > LIMITE_ESQUERDO_X) {                     // enquanto ainda estiver dentro da faixa jogavel
            personagemX = limitar(personagemX + dx, LIMITE_ESQUERDO_X, LIMITE_DIREITO_X); // move o personagem ate o limite
            cameraX = cameraX + dx * VELOCIDADE_CAMERA_DENTRO_FAIXA; // fundo anda pouco enquanto ele caminha
        } else {                                                   // quando ja encostou no limite esquerdo
            cameraX = cameraX + dx * VELOCIDADE_CAMERA_NO_LIMITE;  // fundo anda mais rapido e personagem fica parado
        }
    }
}

/* ===== CALLBACKS ===== */

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {  // recebe teclas
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {                              // ignora soltar tecla
        return;                                                                       // sai sem fazer nada
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {                             // ESC fecha o jogo
        glfwSetWindowShouldClose(window, 1);                                          // pede fechamento da janela
        return;                                                                       // termina callback
    }

    if (key == GLFW_KEY_LEFT) {                                                       // seta esquerda
        moverPersonagem(-PASSO_PERSONAGEM, 0.0f);                                     // move para esquerda
        return;                                                                       // termina callback
    }

    if (key == GLFW_KEY_RIGHT) {                                                      // seta direita
        moverPersonagem(PASSO_PERSONAGEM, 0.0f);                                      // move para direita
        return;                                                                       // termina callback
    }

    if (key == GLFW_KEY_UP) {                                                         // seta para cima
        moverPersonagem(0.0f, -PASSO_PERSONAGEM);                                     // move para cima
        return;                                                                       // termina callback
    }

    if (key == GLFW_KEY_DOWN) {                                                       // seta para baixo
        moverPersonagem(0.0f, PASSO_PERSONAGEM);                                      // move para baixo
        return;                                                                       // termina callback
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {     // recebe cliques do mouse
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {                   // clique esquerdo pressionado
        double mouseX, mouseY;                                                        // posicao do cursor
        glfwGetCursorPos(window, &mouseX, &mouseY);                                   // le a posicao do cursor
        personagemX = limitar((float)mouseX, LIMITE_ESQUERDO_X, LIMITE_DIREITO_X);    // coloca personagem no X clicado dentro da faixa jogavel
        personagemY = limitar((float)mouseY, 80.0f, LIMITE_INFERIOR_Y);                // coloca personagem no Y clicado respeitando o limite inferior
    }
}

/* ===== FUNCAO PRINCIPAL ===== */

int main() {
    glfwInit();                                                                       // inicia GLFW

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                                    // OpenGL 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                                    // OpenGL 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);                    // perfil moderno
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);                                         // janela fixa

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Atividade Vivencial 2", NULL, NULL);
    if (window == NULL) {                                                             // verifica erro ao criar janela
        fprintf(stderr, "ERRO: NAO FOI POSSIVEL CRIAR A JANELA\n");                 // mostra erro
        glfwTerminate();                                                              // encerra GLFW
        return EXIT_FAILURE;                                                          // termina com falha
    }

    glfwMakeContextCurrent(window);                                                   // ativa contexto OpenGL
    glfwSetKeyCallback(window, key_callback);                                         // registra teclado
    glfwSetMouseButtonCallback(window, mouse_button_callback);                        // registra mouse
    glfwSwapInterval(1);                                                              // liga vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {                       // inicializa GLAD
        fprintf(stderr, "ERRO: FALHA AO INICIALIZAR GLAD\n");                       // mostra erro
        glfwTerminate();                                                              // encerra GLFW
        return EXIT_FAILURE;                                                          // termina com falha
    }

    glEnable(GL_BLEND);                                                               // habilita transparencia
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                                // configura transparencia comum

    const char* vertex_shader_textura =                                               // shader de vertices
        "#version 330 core\n"
        "layout (location = 0) in vec3 vp;\n"
        "layout (location = 1) in vec2 vt;\n"
        "uniform mat4 proj;\n"
        "uniform mat4 model;\n"
        "out vec2 text_map;\n"
        "void main(){\n"
        "  text_map = vt;\n"
        "  gl_Position = proj * model * vec4(vp, 1.0);\n"
        "}";

    const char* fragment_shader_textura =                                             // shader de fragmentos
        "#version 330 core\n"
        "in vec2 text_map;\n"
        "uniform sampler2D basic_texture;\n"
        "uniform int flip_vertical;\n"                                               // 1 = inverte a textura verticalmente
        "out vec4 frag_color;\n"
        "void main(){\n"
        "  vec2 uv = text_map;\n"                                                     // copia as coordenadas de textura
        "  if (flip_vertical == 1) uv.y = 1.0 - uv.y;\n"                            // inverte eixo V se necessario
        "  vec4 cor = texture(basic_texture, uv);\n"
        "  if (cor.a < 0.05) discard;\n"
        "  frag_color = cor;\n"
        "}";

    GLuint vsTextura = glCreateShader(GL_VERTEX_SHADER);                              // cria vertex shader
    glShaderSource(vsTextura, 1, &vertex_shader_textura, NULL);                       // envia codigo fonte
    glCompileShader(vsTextura);                                                       // compila shader
    if (!checarShader(vsTextura, "VERTEX TEXTURA")) {                                // confere erro
        glfwTerminate();                                                              // encerra GLFW
        return EXIT_FAILURE;                                                          // termina com falha
    }

    GLuint fsTextura = glCreateShader(GL_FRAGMENT_SHADER);                            // cria fragment shader
    glShaderSource(fsTextura, 1, &fragment_shader_textura, NULL);                     // envia codigo fonte
    glCompileShader(fsTextura);                                                       // compila shader
    if (!checarShader(fsTextura, "FRAGMENT TEXTURA")) {                              // confere erro
        glfwTerminate();                                                              // encerra GLFW
        return EXIT_FAILURE;                                                          // termina com falha
    }

    GLuint programaTextura = glCreateProgram();                                       // cria programa de shader
    glAttachShader(programaTextura, vsTextura);                                       // adiciona vertex shader
    glAttachShader(programaTextura, fsTextura);                                       // adiciona fragment shader
    glLinkProgram(programaTextura);                                                   // linka programa
    if (!checarPrograma(programaTextura, "TEXTURA")) {                               // confere erro
        glfwTerminate();                                                              // encerra GLFW
        return EXIT_FAILURE;                                                          // termina com falha
    }

    float vertices[] = {                                                              // quadrado base com coordenadas de textura
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f
    };

    GLuint VAO, VBO;                                                                 // identificadores do VAO e VBO
    glGenVertexArrays(1, &VAO);                                                       // cria VAO
    glGenBuffers(1, &VBO);                                                            // cria VBO
    glBindVertexArray(VAO);                                                           // ativa VAO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                               // ativa VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);        // envia vertices para GPU

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);      // atributo de posicao
    glEnableVertexAttribArray(0);                                                     // habilita posicao
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);                                                     // habilita textura
    glBindVertexArray(0);                                                             // desativa VAO

    GLuint texturaCeu;                                                                // textura do ceu (camada mais distante)
    GLuint texturaNuvens;                                                             // textura das nuvens (camada intermediaria)
    GLuint texturaMontanhas;                                                          // textura das montanhas (camada proxima)
    GLuint texturaChao;                                                               // textura do chao pixelWall (camada mais a frente)
    GLuint texturaPersonagem;                                                         // textura do personagem

    if (!carregarTexturaRelativaAoFonte("../assets/parallax/sky.png", &texturaCeu)) {
        glfwTerminate();                                                              // encerra em erro
        return EXIT_FAILURE;                                                          // termina com falha
    }

    if (!carregarTexturaRelativaAoFonte("../assets/parallax/clouds.png", &texturaNuvens)) {
        glfwTerminate();                                                              // encerra em erro
        return EXIT_FAILURE;                                                          // termina com falha
    }

    if (!carregarTexturaRelativaAoFonte("../assets/parallax/mountains.png", &texturaMontanhas)) {
        glfwTerminate();                                                              // encerra em erro
        return EXIT_FAILURE;                                                          // termina com falha
    }

    if (!carregarTexturaRelativaAoFonte("../assets/tex/pixelWall.png", &texturaChao)) {
        glfwTerminate();                                                              // encerra em erro
        return EXIT_FAILURE;                                                          // termina com falha
    }

    if (!carregarTexturaRelativaAoFonte("../assets/Modelos3D/unnamed.png", &texturaPersonagem)) {
        glfwTerminate();                                                              // encerra em erro
        return EXIT_FAILURE;                                                          // termina com falha
    }

    Camada camadaCeu       = { texturaCeu,       900.0f, 600.0f, 300.0f, 0.05f }; // ceu: mais distante, se move muito pouco
    Camada camadaNuvens     = { texturaNuvens,   900.0f, 350.0f, 220.0f, 0.15f }; // nuvens: intermediario lento
    Camada camadaMontanhas  = { texturaMontanhas,900.0f, 420.0f, 350.0f, 0.40f }; // montanhas: intermediario rapido
    Camada camadaChao       = { texturaChao,     420.0f, 150.0f, 545.0f, 0.85f }; // chao: mais proximo, se move mais

    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f); // projecao ortografica em pixels

    printf("=== ATIVIDADE VIVENCIAL 2 - PARALLAX ===\n");                           // texto inicial
    printf("SETAS: movem o personagem\n");                                           // ajuda do teclado
    printf("MOUSE ESQUERDO: coloca o personagem no clique\n");                       // ajuda do mouse
    printf("ESC: sair\n\n");                                                         // ajuda para sair

    while (!glfwWindowShouldClose(window)) {                                          // loop principal
        glfwPollEvents();                                                             // processa eventos

        glViewport(0, 0, WIDTH, HEIGHT);                                              // usa a tela toda
        glClearColor(0.42f, 0.68f, 0.90f, 1.0f);                                      // cor base azul
        glClear(GL_COLOR_BUFFER_BIT);                                                 // limpa a tela

        glUseProgram(programaTextura);                                                // usa shader de textura
        glUniformMatrix4fv(glGetUniformLocation(programaTextura, "proj"),            // envia projecao
                           1, GL_FALSE, glm::value_ptr(proj));

        desenharCamada(programaTextura, VAO, camadaCeu);                              // desenha ceu (mais distante)
        desenharCamada(programaTextura, VAO, camadaNuvens);                           // desenha nuvens
        desenharCamada(programaTextura, VAO, camadaMontanhas);                        // desenha montanhas
        desenharCamada(programaTextura, VAO, camadaChao);                             // desenha chao (mais proximo)

        desenharQuad(programaTextura, VAO, texturaPersonagem,                         // desenha personagem
                     personagemX, personagemY, TAMANHO_PERSONAGEM, TAMANHO_PERSONAGEM, 1); // flip=1 corrige a inversao causada pela projecao Y-para-baixo

        glfwSwapBuffers(window);                                                      // mostra o quadro na janela
    }

    glDeleteTextures(1, &texturaCeu);                                                 // apaga textura do ceu
    glDeleteTextures(1, &texturaNuvens);                                              // apaga textura das nuvens
    glDeleteTextures(1, &texturaMontanhas);                                           // apaga textura das montanhas
    glDeleteTextures(1, &texturaChao);                                                // apaga textura do chao
    glDeleteTextures(1, &texturaPersonagem);                                          // apaga textura do personagem
    glDeleteVertexArrays(1, &VAO);                                                    // apaga VAO
    glDeleteBuffers(1, &VBO);                                                         // apaga VBO
    glDeleteProgram(programaTextura);                                                 // apaga programa
    glDeleteShader(vsTextura);                                                        // apaga vertex shader
    glDeleteShader(fsTextura);                                                        // apaga fragment shader

    glfwTerminate();                                                                  // encerra GLFW
    return EXIT_SUCCESS;                                                              // termina com sucesso
}