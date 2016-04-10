#ifndef MESSAGE_H
#define MESSAGE_H

class Message
{
public:
    Message();
    ~Message();
    unsigned char getType();
<<<<<<< HEAD
    virtual char* serialize();
    
=======

private:
>>>>>>> master
    unsigned char type;
    int size;
};

#endif // MESSAGE_H
