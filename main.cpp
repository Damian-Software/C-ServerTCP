#include <iostream>
#include <thread>
#include <chrono>
#include "Network.h"
#include "Packet.h"
#include "SendData.h"
#include "ReadData.h"


// Tutori�l: Jak p�idat nov� paket do syst�mu
// Krok 1: P�id�n� nov� struktury paketu do Packet.h
// Pokud chcete p�idat nov� typ paketu, mus�te nejprve vytvo�it odpov�daj�c� strukturu v souboru Packet.h.
// Nezapome�te tak� p�idat nov� typ do v��tu (enum) PacketType, abyste mohli nov� paket spr�vn� identifikovat.
// Krok 2: Implementace serializace a deserializace v Serialize.cpp
// Po p�id�n� nov� struktury je nutn� implementovat proces serializace a deserializace.
// V souboru Serialize.cpp p�idejte k�d pro serializaci (p�evod dat paketu do bajtov� reprezentace)
// a deserializaci (p�evod bajtov� reprezentace zp�t do struktury paketu).
// Krok 3: Napln�n� struktury a odesl�n� paketu
// Jakmile m�te novou strukturu a jej� serializaci implementovanou, m��ete ji naplnit daty a odeslat pomoc� metody sendPacket().
// Krok 4: Zpracov�n� p�ijat�ho paketu v Network::handleClient()
// Pokud o�ek�v�te, �e server nebo klient budou p�ij�mat nov� typ paketu, p�idejte zpracov�n� tohoto typu
// do funkce Network::handleClient(). Pomoc� deserializace p�e�t�te data a n�sledn� zpracujte obsah podle logiky va�eho programu.


// Funkce pro odes�l�n� zpr�v konkr�tn�mu klientovi
void sendMessageToClient(Network& server, int client_socket, const std::string& message)
{
    MessagePacket message_packet;
    message_packet.message = message;

    // Odesl�n� paketu konkr�tn�mu klientovi
    server.sendPacket(message_packet, client_socket);
    std::cout << "Message packet sent to client socket " << client_socket << ": " << message << std::endl;
}

int main()
{
    // 1. Inicializace serveru a spu�t�n�
    Network server(13000);  // Vytvo��me server naslouchaj�c� na portu 13000
    server.startServer();   // Spust�me server, aby za�al p�ij�mat p�ipojen� klient�

    std::cout << "Server is running and waiting for clients..." << std::endl;

    // 2. Zat�mco server b��, m��eme mu dynamicky pos�lat zpr�vy p�ipojen�m klient�m
    while (true)
    {
        // Po�k�me chv�li, abychom simulovali serverov� chod (nap��klad p�ij�m�n� nov�ch klient�)
        std::this_thread::sleep_for(std::chrono::seconds(10));   // Server m��e d�lat dal�� �kony mezi �ek�n�m na p�ipojen�

    }

    return 0;
}
