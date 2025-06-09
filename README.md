# 🧠 Eliza - Seu Psicólogo de Bolso Pós-Eventos Extremos

![Eliza Project Banner](https://img.shields.io/badge/Status-Conclu%C3%ADdo-brightgreen)
![ThingSpeak Integration](https://img.shields.io/badge/ThingSpeak-Integrado-blue)
![PlatformIO](https://img.shields.io/badge/PlatformIO-VS_Code-orange)
![Wokwi Simulator](https://img.shields.io/badge/Wokwi-Simulator-purple)

---

## 🎯 Visão Geral do Projeto

Este projeto, desenvolvido como parte do nosso trabalho semestral **"Global Solution"**, aborda a crucial questão da **mitigação dos impactos psicológicos em vítimas de eventos extremos** (como tornados, inundações, terremotos, etc.). Entendemos que, além da resposta física, o apoio emocional e psicológico imediato é fundamental para a recuperação.

**Eliza** é um sistema interativo baseado em ESP32 que atua como um "psicólogo de bolso" preliminar. Através de um **keypad matricial** e uma tela TFT, as vítimas podem acessar um chatbot de inteligência artificial (integrado com a API Gemini) que oferece suporte inicial, informações e opções de conversas guiadas para ajudar a navegar seus sentimentos e encontrar recursos. O sistema também registra dados importantes para monitoramento e análise através da plataforma ThingSpeak, permitindo que profissionais acompanhem a demanda e os tipos de tópicos mais procurados. A ideia é ser uma solução barata para amenizar a situação extrema que as pessoas estão vivenciando, todo fluxo é feito inglês para englobar o mundo inteiro.

---

## ✨ Recursos Principais

* **Interação por Keypad Matricial:** Uma interface simples e intuitiva para selecionar opções.
* **Display TFT:** Uma tela colorida que exibe as opções de tópicos, respostas da IA e mensagens de status.
* **LED Verificador de Wi-Fi:** Um LED que indica o status da conexão Wi-Fi.
* **Integração Gemini AI:** Utiliza a API Gemini (Google AI) para fornecer respostas empáticas e sugestões psicológicas.
* **Monitoramento ThingSpeak:** Envia dados relevantes da interação para a plataforma ThingSpeak para análise de telemetria.
* **Conectividade Wi-Fi:** Permite a comunicação com a API Gemini e ThingSpeak.

---

## 👥 Desenvolvedores

* **Samuel Yariwake** - RM5556461
* **Luiz Felipe** - RM5555591
* **Gabriel Lemos** - RM554819

---

## 🚀 Tecnologias e Bibliotecas Utilizadas

Este projeto foi desenvolvido utilizando a robustez do **ESP32** para conectividade e processamento, e as seguintes ferramentas e bibliotecas:

* **Wokwi Extension for VS Code:** Permitindo a simulação e depuração do hardware e do código de forma eficiente antes da implementação física.
* **PlatformIO Extension for VS Code:** Ambiente de desenvolvimento unificado para microcontroladores, facilitando a compilação, upload e gerenciamento de bibliotecas. O preset utilizado foi `DOIT ESP32 DEVKIT V1`.
* **`WiFi.h`:** Biblioteca padrão para gerenciar a conexão Wi-Fi do ESP32.
* **`HTTPClient.h`:** Utilizada para fazer requisições HTTP para serviços web externos como ThingSpeak e Gemini.
* **`ArduinoJson.h`:** Essencial para o parsing (leitura) e serialização (escrita) de dados no formato JSON, crucial para a comunicação com a API Gemini.
* **`Adafruit_GFX.h`:** Biblioteca gráfica fundamental para desenhar formas e texto na tela TFT.
* **`Adafruit_ILI9341.h`:** Driver específico para o display TFT ILI9341, permitindo a exibição das informações.
* **`Keypad.h`:** Facilita a leitura de entradas de um keypad matricial.

---

## 📈 Integração com ThingSpeak

O ThingSpeak é utilizado para coletar e visualizar dados importantes sobre o uso do sistema Eliza. Isso nos permite entender quais tópicos estão sendo mais acessados e a performance das requisições à IA. Os dados são enviados em lote para otimizar o uso da API e respeitar os limites de taxa do ThingSpeak.

![image](https://github.com/user-attachments/assets/cd71696e-e668-470f-ab86-ffa0004ddfe9)

### Campos do ThingSpeak:

* **Field 1 (Topics Count):**
    * **O que pega:** O número de opções de tópicos que a IA do Gemini forneceu ao usuário na última interação. Caso a resposta da IA tenha falhado (`field3_val = 0`), este campo é setado para `-1`.
    * **Por que é importante:** Indica a "variedade" ou a profundidade da resposta da IA. Se a IA frequentemente retorna poucas opções, pode indicar que a consulta foi muito específica ou que o modelo não conseguiu gerar diversas abordagens. O valor `-1` ajuda a identificar interações onde a IA não pôde responder adequadamente.
* **Field 2 (Initial Topic Selected):**
    * **O que pega:** O número correspondente ao tópico inicial que o usuário selecionou no keypad (1-9) ao iniciar a interação. Este campo só é enviado se um tópico inicial for de fato selecionado (`!= 0`).
    * **Por que é importante:** Permite analisar quais temas são mais procurados pelas vítimas, ajudando a identificar as principais preocupações e a ajustar o conteúdo oferecido.
* **Field 3 (Gemini HTTP Code):**
    * **O que pega:** O código de status HTTP retornado pela requisição à API Gemini (ex: `200` para sucesso, `400` para erro de requisição, `500` para erro no servidor da IA).
    * **Por que é importante:** É crucial para monitorar a saúde e a disponibilidade da integração com a IA. Códigos de erro podem indicar problemas de rede, limites de API atingidos ou falhas no serviço da Gemini.

---

## 🔌 Esquema de Conexão (Hardware)

Para replicar este projeto, você precisará conectar o ESP32 a um display TFT ILI9341 e um keypad matricial 4x3.

### Conexões Chave:

**Display TFT ILI9341:**

* **TFT_DC (Data/Command):** `GPIO 2`
* **TFT_CS (Chip Select):** `GPIO 15`
* **LED (Backlight):** `GPIO 16` (para controle do brilho)
* **SCLK (Clock):** `GPIO 18` (Padrão SPI do ESP32 - VSPI CLK)
* **MOSI (Data In):** `GPIO 23` (Padrão SPI do ESP32 - VSPI MOSI)
* **MISO (Data Out):** `GPIO 19` (Padrão SPI do ESP32 - VSPI MISO - nem sempre necessário para display, mas bom para consistência)
* **VCC:** `3.3V` (do ESP32)
* **GND:** `GND` (do ESP32)

**Keypad Matricial 4x3:**

O keypad é configurado com as seguintes conexões:

* **Colunas (`colPins`):**
    * `Col 1`: `GPIO 12`
    * `Col 2`: `GPIO 13`
    * `Col 3`: `GPIO 17`
* **Linhas (`rowPins`):**
    * `Row 1`: `GPIO 14`
    * `Row 2`: `GPIO 27`
    * `Row 3`: `GPIO 26`
    * `Row 4`: `GPIO 25`

**LED Verificador de Wi-Fi:**

* **LED:** `GPIO 16` (Compartilhado com o Backlight do TFT, indicando Wi-Fi conectado quando aceso)

**Observação:** Certifique-se de que todas as conexões estejam firmes e que os pinos de alimentação (VCC/GND) estejam corretos para evitar danos aos componentes. Para o **DOIT ESP32 DEVKIT V1**, os pinos GPIO mencionados são geralmente seguros para uso geral.

---

## 🛠️ Como Utilizar o Projeto

1.  **Clone o Repositório:**
    ```bash
    git clone [https://github.com/seu-usuario/Eliza-Psicologo-de-Bolso.git](https://github.com/seu-usuario/Eliza-Psicologo-de-Bolso.git)
    cd Eliza-Psicologo-de-Bolso
    ```
2.  **Abra no VS Code com PlatformIO:**
    * Certifique-se de ter as extensões **PlatformIO IDE** e **Wokwi for ESP32** instaladas no VS Code.
    * Abra a pasta clonada (`Eliza-Psicologo-de-Bolso`) no VS Code. O PlatformIO deve reconhecer automaticamente o projeto.
3.  **Configurações e Credenciais:**
    * O código fornecido já inclui as credenciais para o Wi-Fi `Wokwi-GUEST` e a API Gemini, além do ThingSpeak. Se você estiver usando hardware físico ou uma rede diferente, ou se suas chaves mudarem, precisará atualizar essas informações diretamente no `main.cpp` (ou `.ino`):
        ```cpp
        const char* ssid = "Wokwi-GUEST"; // Mude para o seu SSID se não estiver no Wokwi
        const char* password = ""; // Mude para a sua senha Wi-Fi se não estiver no Wokwi
        const char* Gemini_Token = "***"; // Sua chave da API Gemini
        unsigned long channelID = ****; // Seu ID do canal ThingSpeak
        const char* writeAPIKey = "***"; // Sua chave de escrita da API ThingSpeak
        ```
        **Lembre-se: Mantenha suas chaves de API privadas e nunca as compartilhe publicamente.**
4.  **Instale as Dependências:**
    * O PlatformIO deve sugerir a instalação das bibliotecas listadas no `platformio.ini`. Se não, instale-as manualmente através do Library Manager do PlatformIO:
        * `ArduinoJson`
        * `Adafruit GFX Library`
        * `Adafruit ILI9341`
        * `Keypad`
Após isso é só buildar e testar clicando no diagram.json e no botão de play!
5.  **Simulação (Opcional, mas Recomendado):**
    * Use a extensão Wokwi para simular o projeto antes de fazer o upload para o hardware real. Isso ajuda a identificar problemas de hardware e lógica rapidamente.
    * Com o projeto aberto no VS Code, a extensão Wokwi deve ser ativada. Você pode iniciar a simulação diretamente pelo VS Code (procure o botão de "Play" do Wokwi). O diagrama de conexão já está configurado nos projetos Wokwi referenciados.
6.  **Upload para o ESP32:**
    * Conecte seu ESP32 (preferencialmente um `DOIT ESP32 DEVKIT V1`) ao computador via USB.
    * No PlatformIO, selecione a porta serial correta (geralmente `/dev/ttyUSB0` no Linux/macOS ou `COMx` no Windows).
    * Clique no botão "Upload" (seta para a direita) na barra de ferramentas do PlatformIO para compilar e carregar o código para o seu ESP32.
7.  **Interaja:**
    * Após o upload, o display TFT deve inicializar e mostrar as opções de tópicos.
    * Use o **keypad** para selecionar uma opção (1-9) ou as teclas de controle (H, W, P).
        * `H`: Retorna à tela inicial de tópicos.
        * `W`: Tenta reconectar ao Wi-Fi ou desconecta, dependendo do estado atual. O LED de Wi-Fi (`GPIO 16`) indicará o status.
        * `P`: Reinicia o dispositivo.
    * As respostas da IA serão exibidas na tela, e os dados serão enviados para o ThingSpeak para monitoramento.
    * Tópicos iniciais:
      - "Coping with anxiety",
      - "Managing stress",
      - "Dealing with sadness",
      - "Improving sleep",
      - "Handling loneliness",
      - "Building self-confidence",
      - "Overcoming fear",
      - "Finding motivation",
      - "Talking about a recent extreme event"

---

## 🔗 Links de Referência

* [Wokwi Project 1](https://wokwi.com/projects/375237011181407233)
* [Wokwi Project 2](https://wokwi.com/projects/320964045035274834)
* [Wokwi Project 3](https://wokwi.com/projects/403362284570337281)
* [Wokwi Project 4](https://wokwi.com/projects/370824465277751297)
* [Wokwi Project 5](https://wokwi.com/projects/294980637632233994)
* [Wokwi Project 6](https://wokwi.com/projects/342032431249883731)
