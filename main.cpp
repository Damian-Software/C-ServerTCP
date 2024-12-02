#include <iostream>
#include <thread>
#include <chrono>
#include "Network.h"
#include "Packet.h"
#include "SendData.h"
#include "ReadData.h"


// Tutoriál: Jak pøidat nový paket do systému
// Krok 1: Pøidání nové struktury paketu do Packet.h
// Pokud chcete pøidat nový typ paketu, musíte nejprve vytvoøit odpovídající strukturu v souboru Packet.h.
// Nezapomeòte také pøidat nový typ do výètu (enum) PacketType, abyste mohli nový paket správnì identifikovat.
// Krok 2: Implementace serializace a deserializace v Serialize.cpp
// Po pøidání nové struktury je nutné implementovat proces serializace a deserializace.
// V souboru Serialize.cpp pøidejte kód pro serializaci (pøevod dat paketu do bajtové reprezentace)
// a deserializaci (pøevod bajtové reprezentace zpìt do struktury paketu).
// Krok 3: Naplnìní struktury a odeslání paketu
// Jakmile máte novou strukturu a její serializaci implementovanou, mùžete ji naplnit daty a odeslat pomocí metody sendPacket().
// Krok 4: Zpracování pøijatého paketu v Network::handleClient()
// Pokud oèekáváte, že server nebo klient budou pøijímat nový typ paketu, pøidejte zpracování tohoto typu
// do funkce Network::handleClient(). Pomocí deserializace pøeètìte data a následnì zpracujte obsah podle logiky vašeho programu.


// Funkce pro odesílání zpráv konkrétnímu klientovi
void sendMessageToClient(Network& server, int client_socket, const std::string& message)
{
    MessagePacket message_packet;
    message_packet.message = message;

    // Odeslání paketu konkrétnímu klientovi
    server.sendPacket(message_packet, client_socket);
    std::cout << "Message packet sent to client socket " << client_socket << ": " << message << std::endl;
}

int main()
{
    // 1. Inicializace serveru a spuštìní
    Network server(13000);  // Vytvoøíme server naslouchající na portu 13000
    server.startServer();   // Spustíme server, aby zaèal pøijímat pøipojení klientù

    std::cout << "Server is running and waiting for clients..." << std::endl;

    // 2. Zatímco server bìží, mùžeme mu dynamicky posílat zprávy pøipojeným klientùm
    while (true)
    {
        // Poèkáme chvíli, abychom simulovali serverový chod (napøíklad pøijímání nových klientù)
        std::this_thread::sleep_for(std::chrono::seconds(10));   // Server mùže dìlat další úkony mezi èekáním na pøipojení

    }

    return 0;
}
