
```
int Chip8::LoadRom(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streampos size;
    if (file.is_open())
    {
        size = file.tellg();
        char *buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        std::ios init(NULL);
        init.copyfmt(std::cout);
        for (long i = 0; i < size; ++i)
        {
            memory[START_ADDRESS + i] = buffer[i];

            std::cout << std::hex << START_ADDRESS + i << ": ";
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << ((+buffer[i]) & 0xFFu);
            std::cout << "\n";
        }
        std::cout.copyfmt(init);
        delete[] buffer;
    }
    return size;
}
```