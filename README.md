# Sistema de Streaming KERNELPOP de Manipulação Direta NO DISCO :movie_camera: :computer: 📺 🐧

Bem-vindo ao sistema de streaming KernelPop!!! Esse projeto é um gerenciador de filmes feito totalmente em C. A grande sacada aqui é manipulação direta no disco.
Chega de arrays gigantes comendo toda a sua memória RAM. O sistema lê, escreve e atualiza os bytes direto no arquivo físico .dat

Quais são suas funcionalidades?
[C]reate: Cadastra seus filmes. Dá para colocar de tudo, desde um mistério denso na vibe Agatha Christie até uma comédia farofa do Adam Sandler.

[R]ead: Busca o filme pelo ID. Achou? Ele te mostra tudo: diretor, ano, duração, nota e gênero.

[U]pdate: Escreveu o nome do diretor errado ou quer mudar a nota? Dá para atualizar o registro. O sistema acha o filme, volta o ponteiro do disco no lugar exato e sobrescreve as informações, forçando a Chave Primária (ID) a continuar a mesma.

[D]elete (A remoção fantasma): A remoção é lógica. O sistema não apaga os bytes e bagunça o arquivo, ele só liga uma flag (removido = '1'). O filme continua lá no HD, mas o sistema finge que não vê.

Alguns Detalhes Técnicos

   💻Linguagem: C puro.

   🏗️ Tamanho do Registro: Exatos 212 bytes da struct Filme.

   📁 Navegação no Arquivo: Feita na base do fseek, fread e fwrite. O arquivo nunca sobe inteiro pra memória, o código só puxa ou empurra um bloquinho de 212 bytes por vez.

   🆔Integridade: O sistema trava a inserção de Chaves Primárias (IDs) repetidas.

Como rodar na sua máquina?

Se você tem o compilador (GCC) instalado, é só abrir o terminal e roda

    Compile os arquivos juntos:
    Bash

    gcc main.c filme.c -o streaming

    Rode o executável:

    streaming.exe

        No Linux/Mac: ./streaming

    O menu vai aparecer no terminal. Se o arquivo dados_filmes.dat não existir, relaxa que o próprio programa cria na hora.
