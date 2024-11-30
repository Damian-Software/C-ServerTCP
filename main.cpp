#include <iostream>
#include "Network.h"
#include "Packet.h"


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


//int main(int argc, char* argv[])
//{
//    // Initialize the network client
//    Network client("192.168.56.1", 13000);
//    client.connect();
//
//    // Create a packet to send
//    LoginPacket packet;
//    packet.account_id = 12345;
//    packet.password = "password123";
//
//    // Send the packet
//    client.sendPacket(packet);
//
//    // Create and send a message packet
//    MessagePacket message_packet;
//    message_packet.message = "Ahoj svete";
//    client.sendPacket(message_packet);
//
//    //std::vector<uint8_t> screenshot_data = takeScreenshot();  // Funkce pro z�sk�n� screenshotu
//    //ScreenPacket screen_packet;
//    //screen_packet.screen_data = screenshot_data;
//    //client.sendPacket(screen_packet);
//
//    // Create and send a data packet
//    DataPacket data_packet;
//    data_packet.data = { 'H', 'e', 'l', 'l', 'o' };
//    data_packet.data_as_string = "Hello";
//    client.sendPacket(data_packet);
//
//    // Create and send a test packet
//    TestPacket test_packet;
//    test_packet.test_string = "Test string";
//    test_packet.test_vector = { 1, 2, 3, 4, 5 };
//    client.sendPacket(test_packet);
//
//    // Process incoming packets (this could be done in a loop (while) if needed)
//    client.processIncomingPackets();
//
//
//
//    return 0;
//
//}
//


int main()
{
    Network server(13000);
    server.startServer();

    return 0;
}

