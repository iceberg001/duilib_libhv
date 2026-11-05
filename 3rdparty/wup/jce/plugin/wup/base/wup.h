#ifndef _WUP_H_
#define _WUP_H_
#include <map>
#include <string>
#include <vector>
#include <sstream>

//支持iphone
#ifdef __APPLE__
#include "BasePacketF.h"
#elif defined ANDROID  // android
#include "BasePacketF.h"
#elif defined WIN32  // windows
#include "BasePacketF.h"
#else
#include "wup-linux-c++/BasePacketF.h"
#endif

#ifdef __GNUC__
#if __GNUC__ >3 || __GNUC_MINOR__ > 3
#include <ext/pool_allocator.h>
#endif
#endif

using namespace std;
using namespace taf;

namespace wup
{

/////////////////////////////////////////////////////////////////////////////////
// 属性封装类

template<typename TWriter = BufferWriter, typename TReader = BufferReader, template<typename> class Alloc = std::allocator>
//template<typename> class Alloc = __gnu_cxx::__pool_alloc >
class UniAttribute
{
    typedef vector<char, Alloc<char> > VECTOR_CHAR_TYPE;
    typedef map<string, VECTOR_CHAR_TYPE, less<string>, Alloc<pair<string, VECTOR_CHAR_TYPE> > > VECTOR_CHAR_IN_MAP_TYPE;

public:
    /**
     * 构造函数
     */
    UniAttribute()
    {
    }

    /**
     * 添加属性值
     *
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值
     */
    template<typename T> void put(const string& name, const T& t)
    {
        os.reset();

        os.write(t, 0);

        VECTOR_CHAR_TYPE& v = _data[name];
        v.assign(os.getBuffer(), os.getBuffer() + os.getLength());
    }

    /**
     * 获取属性值，属性不存在则抛出异常
     *
     * @throw runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值输出参数
     */
    template<typename T> void get(const string& name, T& t)
    {
        typename VECTOR_CHAR_IN_MAP_TYPE::iterator mit;

        mit = _data.find(name);

        if (mit != _data.end())
        {
            is.reset();

            is.setBuffer(mit->second);

            is.read(t, 0, true);

            return;

        }
        throw runtime_error(string("UniAttribute not found key:") +  name);
    }
    /**
     * 获取属性值，属性不存在则抛出异常
     *
     * @throw runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @return T:  属性值
     */
    template<typename T> T get(const string& name)
    {
        T t;

        get<T>(name, t);

        return t;
    }
    /**
     * 获取属性值，忽略异常，不存在的属性返回缺省值
     *
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:  属性值输出参数
     * @param def:     默认值
     */
    template<typename T> void getByDefault(const string& name, T& t, const T& def)
    {
        try
        {
            get<T>(name, t);
        }
        catch (runtime_error& e)
        {
            t = def;
        }
    }
    /**
     * 获取属性值(忽略异常，def为缺省值)
     *
     * @param T:   属性类型
     * @param name:属性名称
     * @param:     默认值
     * @return T:  属性值
     */
    template<typename T> T getByDefault(const string& name, const T& def)
    {
        T t;

        getByDefault<T>(name, t, def);

        return t;
    }

    /**
     *清除全部属性值
     */
    void clear() { _data.clear(); }

    /** 编码
     *
     * @param buff： 编码结果输出参数
     */
    void encode(string& buff)
    {
        os.reset();

        os.write(_data, 0);

        buff.assign(os.getBuffer(), os.getLength());
    }

    /** 编码
     *
     * @param buff： 编码结果输出参数
     */
    void encode(vector<char>& buff)
    {
        os.reset();

        os.write(_data, 0);

        buff.assign(os.getBuffer(), os.getBuffer() + os.getLength());
    }

    /** 编码
     *
     * @throw runtime_error
     * @param buff：输出存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char *buff, size_t& len)
    {
        os.reset();

        os.write(_data, 0);

        if (len < os.getLength()) throw runtime_error("encode error, buffer length too short");
        memcpy(buff, os.getBuffer(), os.getLength());
        len =  os.getLength();
    }

    /** 解码
     *
     * @throw runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */
    void decode(const char *buff, size_t len)
    {
        is.reset();

        is.setBuffer(buff, len);

        _data.clear();

        is.read(_data, 0, true);
    }

    /**
     * 解码
     *
     * @throw runtime_error
     * @param buff： 待解码的字节流
     */
    void decode(const vector<char>& buff)
    {
        is.reset();

        is.setBuffer(buff);
        _data.clear();

        is.read(_data, 0, true);
    }

    const VECTOR_CHAR_IN_MAP_TYPE& getData() const { return _data; }

    /**
     * 判断属性集合是否为空
     *
     * @return bool:属性是否为空
     */
    bool isEmpty() { return _data.empty(); }

    /**
     * 获取属性集合大小
     *
     * @return size_t:  集合大小
     */
    size_t size() { return _data.size(); }

    /**
     * 判断属性是否存在
     *
     * @param key:属性名称
     * @return bool:是否存在
     */
    bool containsKey(const string& key) { return _data.find(key) != _data.end(); }

//protected:
public:
    VECTOR_CHAR_IN_MAP_TYPE _data;
//    short _iVer;

public:
    JceInputStream<TReader>     is;
    JceOutputStream<TWriter>    os;
};

/////////////////////////////////////////////////////////////////////////////////
// 请求、回应包封装类

template<typename TWriter = BufferWriter, typename TReader = BufferReader, template<typename> class Alloc = std::allocator>
struct UniPacket : protected BasePacket, public UniAttribute<TWriter, TReader, Alloc>
{
private:
	UniPacket(const UniPacket& wup);
	UniPacket& operator=(const UniPacket& buf);
public:
    /**
     * 构造函数
     */
    UniPacket()
    {
        iVersion = 2; cPacketType = 0;

        iMessageType = 0; iRequestId = 0;

        sServantName = ""; sFuncName = "";

        iTimeout = 0; sBuffer.clear(); iRet = 0; sResultDesc = "";

        context.clear(); status.clear();

        UniAttribute<TWriter, TReader, Alloc>::_data.clear();
    }

    /**
     * 拷贝构造
     * @param wup
     */
    //UniPacket(const UniPacket& wup)  { *this = wup;}

    /**
     * 由请求包生成回应包基本结构，回填关键的请求信息
     *
     * @return UniPacket： 回应包
     */
    UniPacket createResponse()
    {
        UniPacket respPacket;

        respPacket.sServantName = sServantName;
        respPacket.sFuncName    = sFuncName;
        respPacket.iRequestId   = iRequestId;

        return respPacket;
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     *
     * @throw runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(string& buff)
    {
        JceOutputStream<TWriter>& os = UniAttribute<TWriter, TReader, Alloc>::os;

        os.reset();

        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());
        buff.assign((const char *)&iHeaderLen, sizeof(taf::Int32));

        buff.append(os.getBuffer(), os.getLength());
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     *
     * @throw runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(vector<char>& buff)
    {
        JceOutputStream<TWriter>& os = UniAttribute<TWriter, TReader, Alloc>::os;

        os.reset();

        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());

        buff.resize(sizeof(taf::Int32) + os.getLength());
        memcpy(&buff[0], &iHeaderLen, sizeof(taf::Int32));
        memcpy(&buff[sizeof(taf::Int32)], os.getBuffer(), os.getLength());

    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * @throw runtime_error
     * @param buff：存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char *buff, size_t& len)
    {
        JceOutputStream<TWriter>& os = UniAttribute<TWriter, TReader, Alloc>::os;

        os.reset();

        doEncode(os);

        taf::Int32 iHeaderLen = htonl(sizeof(taf::Int32) + os.getLength());
        if (len < sizeof(taf::Int32) + os.getLength()) throw runtime_error("encode error, buffer length too short");

        memcpy(buff, &iHeaderLen, sizeof(taf::Int32));
        memcpy(buff + sizeof(taf::Int32), os.getBuffer(), os.getLength());

        len = sizeof(taf::Int32) + os.getLength();
    }

    /** 解码
     *
     * @throw runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */

    void decode(const char *buff, size_t len)
    {
        if (len < sizeof(taf::Int32)) throw runtime_error("packet length too short");

        JceInputStream<TReader> &is = UniAttribute<TWriter, TReader, Alloc>::is;

        is.reset();

        is.setBuffer(buff + sizeof(taf::Int32), len - sizeof(taf::Int32));

        readFrom(is);

        is.reset();

        is.setBuffer(sBuffer);

        UniAttribute<TWriter, TReader, Alloc>::_data.clear();

        is.read(UniAttribute<TWriter, TReader, Alloc>::_data, 0, true);
    }

public:
    /**
     * 获取调用taf的返回值
     *
     * @retrun taf::Int32
     */
    taf::Int32 getResultCode() const { return this->iRet; }

    /**
     * 设置调用taf返回值
     *
     * @retrun
     */
    void setResultCode(taf::Int32 iRet)  { this->iRet = iRet; }

    /**
     * 获取调用taf的返回描述
     *
     * @retrun string
     */
    string getResultDesc() const { return this->sResultDesc; }

    /**
     * 获取上下文信息
     * @return const map<std::string,std::string>&
     */
    const map<std::string, std::string>& getContext() const { return this->context; }

    /**
     * 获取特殊消息的状态值
     * @return const map<std::string,std::string>&
     */
    const map<std::string, std::string>& getStatus() const { return this->status; }

    /**
     * 获取消息version
     * @return taf::Short
     */
    taf::Short getVersion() const { return iVersion; }

    /**
     * 设置版本
     *
     * @param iVer
     */
    void setVersion(taf::Short iVer) { iVersion = iVer; }

    /**
     * 获取消息ID
     * @return taf::Int32
     */
    taf::Int32 getRequestId() const { return iRequestId; }
    /**
     * 设置请求ID
     * @param value
     */
    void setRequestId(taf::Int32 value) { iRequestId = value; }
    /**
     * 获取对象名称
     * @return const std::string&
     */
    const std::string& getServantName() const { return sServantName; }
    /**
     * 设置对象名称
     * @param value
     */
    void setServantName(const std::string& value) { sServantName = value; }
    /**
     * 获取方法名
     * @return const std::string&
     */
    const std::string& getFuncName() const { return sFuncName; }
    /**
     * 设置方法名
     * @param value
     */
    void setFuncName(const std::string& value) { sFuncName = value; }

protected:
    /**
     * 内部编码
     */
    void doEncode(JceOutputStream<TWriter>& os)
    {
        //ServantName、FuncName不能为空
        if (sServantName.empty()) throw runtime_error("ServantName must not be empty");
        if (sFuncName.empty())    throw runtime_error("FuncName must not be empty");

        os.reset();

        os.write(UniAttribute<TWriter, TReader, Alloc>::_data, 0);

        sBuffer.assign(os.getBuffer(), os.getBuffer() + os.getLength());

        os.reset();

        writeTo(os);
    }

};


#ifdef __GNUC__
#if __GNUC__ >3 || __GNUC_MINOR__ > 3
typedef UniAttribute<BufferWriter, BufferReader, __gnu_cxx::__pool_alloc> UniAttrPoolAlloc;
typedef UniPacket<BufferWriter, BufferReader, __gnu_cxx::__pool_alloc> UniPacketPoolAlloc;
//typedef TafUniPacket<BufferWriter, BufferReader, __gnu_cxx::__pool_alloc> TafUniPacketPoolAlloc;
#endif
#endif


}
////////////////////////////////////////////////////////////////////////////////////////////////
#endif
