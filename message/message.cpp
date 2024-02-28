#include "message.h"
namespace CoTrain
{

    void BufMessage::showdata()
    {
        std::cout << m_data << std::endl;
    }

    BufMessage::BufMessage()
    {
        m_semaphore = Semaphore::ptr(new Semaphore(0));
        if (m_size == 0)
        {
            m_size = max_buf_len;
        }
        m_data = malloc(m_size);
        if (m_data == nullptr)
        {
            return;
        }
        m_buf = Message::Bufptr(m_data);
        // 初始化
        std::memset(m_buf.get(), 0, m_size);
    }
    BufMessage::BufMessage(uint64_t size)
    {
        m_semaphore = Semaphore::ptr(new Semaphore(0));

        m_size = size;
        m_data = malloc(m_size);
        if (m_data == nullptr)
        {
            return;
        }
        m_buf = Message::Bufptr(m_data);
        // 初始化
        std::memset(m_buf.get(), 0, m_size);
    }

    BufMessage::~BufMessage()
    {
    }
    void BufMessage::toFile(std::string filepath)
    {
    
    }
    std::string BufMessage::toString()
    {
        if(m_size == 0){
            return "";
        }
        std::string s = std::string(static_cast<char*>(m_data), m_size);
        return s;
        // return "";
    }
    void BufMessage::wait()
    {
        m_semaphore->wait();
    }
    void BufMessage::notify()
    {
        m_semaphore->notify();
    }
    ComMessage::ComMessage(ComMessageType::ComType type, uint64_t uniqueID)
    {
        m_messagetype = type;
        switch (type)
        {
        case ComMessageType::ComType::AlIVE:
        case ComMessageType::ComType::CONNECT:
            /* code */
            m_type = static_cast<uint16_t>(m_messagetype);
            m_operation = 0; // 设置操作为0，表示无特定操作
            m_size = sizeof(uint16_t) + sizeof(uint64_t); // 设置消息大小为消息类型的大小
            m_uniqueID = uniqueID; // 设置独特ID编码为0，可以根据需要进行修改
            m_data = std::malloc(max_com_len); // 为数据指针分配内存
            // std::memcpy(m_data, &m_type, sizeof(uint16_t)); // 将消息类型存储在数据指针中
            
            encodeHead();
            // std::cout << (char *)m_data << std::endl;

            std::memcpy(static_cast<char*>(m_data) + sizeof(uint16_t), &m_uniqueID, sizeof(uint64_t));
            break;

        case ComMessageType::ComType::FILE:
            
            m_type = static_cast<uint16_t>(m_messagetype);
            m_operation = 0; // 设置操作为0，表示无特定操作
            m_size = sizeof(uint16_t) + sizeof(uint64_t); // 设置消息大小为消息类型的大小
            m_uniqueID = uniqueID; // 设置独特ID编码为0，可以根据需要进行修改
            m_data = std::malloc(max_com_len); // 为数据指针分配内存
            // std::memcpy(m_data, &m_type, sizeof(uint16_t)); // 将消息类型存储在数据指针中
            
            encodeHead();
            // std::cout << (char *)m_data << std::endl;

            std::memcpy(static_cast<char*>(m_data) + sizeof(uint16_t), &m_uniqueID, sizeof(uint64_t));
            break;
        default:
            break;
        }

        addMessageEnd();
        m_size = max_com_len;
    }
    ComMessage::ComMessage(ComMessageType::ComType type, uint64_t uniqueID, uint64_t bufsize)
    {
        m_messagetype = type;
        switch (type){

        case ComMessageType::ComType::FILE:
            m_filesize = bufsize;
            m_type = static_cast<uint16_t>(m_messagetype);
            m_operation = 0; // 设置操作为0，表示无特定操作
            m_size = sizeof(uint16_t) + sizeof(uint64_t); // 设置消息大小为消息类型的大小
            m_uniqueID = uniqueID; // 设置独特ID编码为0，可以根据需要进行修改
            m_data = std::malloc(max_com_len); // 为数据指针分配内存
            // std::memcpy(m_data, &m_type, sizeof(uint16_t)); // 将消息类型存储在数据指针中
            
            encodeHead();
            // std::cout << (char *)m_data << std::endl;

            std::memcpy(static_cast<char*>(m_data) + sizeof(uint16_t), &m_uniqueID, sizeof(uint64_t));
            break;
            
            default:
            break;

        }
    }
    ComMessage::ComMessage()
    {
        m_size = max_com_len;
        m_data = std::malloc(m_size);

    }
    ComMessage::~ComMessage()
    {
    }
    std::string Message::showheader()
    {
        std::stringstream ss;
        ss << "Message Type: " << m_type << std::endl;
        ss << "Message Operation: " << m_operation << std::endl;
        ss << "Message Size: " << m_size << std::endl;
        ss << "Unique ID: " << m_uniqueID << std::endl;
        return ss.str();
    }
    void Message::encodeHead()
    {
        uint16_t header = 0;

        // 将消息类型存储在头部的1-3位
        header |= (m_type << 13);

        // 将消息操作存储在头部的4-8位
        header |= (m_operation << 8);

        // 将消息大小存储在头部的8-16位
        header |= m_size;
        
        std::memcpy(m_data, &header, sizeof(uint16_t));
    }
    void Message::decodeHead()
    {
        uint16_t header = 0;

        // 从 m_data 的开头复制头部信息
        std::memcpy(&header, m_data, sizeof(uint16_t));

        // 解码头部信息
        m_type = (header >> 13) & 0x07;
        m_operation = (header >> 8) & 0x1F;
        m_size = header & 0xFF;
    }
    void Message::addMessageEnd()
    {
        char end = '#';

        std::memcpy(&end, m_data,m_size);

        m_size += sizeof(end);
    }
}
