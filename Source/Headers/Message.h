#ifndef MESSAGE_H
#define MESSAGE_H

class Message
{
public:
    Message();
    ~Message();
    unsigned char getType();

private:
    unsigned char type;
};

#endif // MESSAGE_H
