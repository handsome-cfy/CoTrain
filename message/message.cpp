#include "message.h"
namespace CoTrain
{

    void BufMessage::showdata()
    {
        std::cout << m_data << std::endl;
    }

    BufMessage::BufMessage()
    {
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
    BufMessage::~BufMessage()
    {
    }
    ComMessage::ComMessage()
    {
    }
    ComMessage::~ComMessage()
    {
    }
}
