#include <iostream>
#include <iomanip>
#include <fstream>

int main()
{
    int i = 0;
    std::ifstream file("Programs/IBMLogo.ch8", std::ios::binary);
    if (file.is_open())
    {
        char x;
        while (file)
        {
            file.read(&x, 1);
            // std::cout << std::dec << x;
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << ((+x) & 0xFFu);
            std::cout << std::dec << " " << i << "\n";
            i++;
        }
        file.close();
    }
    std::cout << "---" << std::dec <<  i << "---";
}