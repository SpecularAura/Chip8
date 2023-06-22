
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
        for (long i = 0; i < size; ++i)
        {
            memory[START_ADDRESS + i] = buffer[i];
        }
        delete[] buffer;
    }
    return size;
}
```
The above snippet works fine


```
int Chip8::LoadRom(const char *filename)
{
    int i = 0;
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open())
    {
        char x;
        while (file)
        {
            file.read(&x, 1);
            memory[START_ADDRESS + i] = x;
            i++;
        }
        file.close();
    }
    return i;
}
```
This works just as fine too

However,
```
int Chip8::LoadRom(const char *filename)
{
    int i = 0;
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open())
    {
        char x;
        while (file >> x)
        {
            memory[START_ADDRESS + i] = x;
            i++;
        }
        file.close();
    }
    return i;
}
```
Skips over some lines and I can't figure out a reason why