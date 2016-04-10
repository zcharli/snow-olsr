#ifndef MESSAGE_H
#define MESSAGE_H

class Message
{
public:
    Message();
    ~Message();
    unsigned char getType();
    virtual char* serialize();
    

private:
    unsigned char type;
    int size;
};

#endif // MESSAGE_H
