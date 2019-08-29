#include "bin_xml.h"
#include "bin_xml_types.h"
#include "bin_write_plugin.h"
#include "macros.h"
#include <string.h>
#include <assert.h>

namespace pklib_xml {

void BW_element::init(int16_t id,int8_t type,int8_t flags,BW_offset_t my_offset)
{
    assert(sizeof(*this) == 20);
    this->identification = id;
    this->value_type = type;
    this->flags = flags;
    this->prev = my_offset;
    this->next = my_offset;
/* This is not necessary, because data are zero filled globally
    this->first_child = 0;
    this->first_attribute = 0;
*/
}
void        BW_element::init(BW_pool *pool,int16_t identificaton,int8_t value_type,int8_t flags);
{
    assert(sizeof(*this) == 24);
    assert(pool != nullptr);

    int offset = reinterpret_cast<char*>(this) - reinterpret_cast<char*>(pool);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= pool->size); 
    this->offset = offset;

    this->identification = identification;
    this->value_type = value_type;
    this->flags = flags;

    this->next = my_offset;
    this->prev = my_offset;
    this->first_child = 0;
    this->first_attribute = 0;
}

BW_pool*    BW_element::getPool() const
{
    return reinterpret_cast<BW_pool*>(reinterpret_cast<char*>(this) - offset);
}

BW_element* BW_element::BWE(BW_offset_t offset) const
{
    return reinterpret_cast<BW_element*>(reinterpret_cast<char*>(this) - this->offset + offset);
}
char*       BW_element::BWD() const
{
    return reinterpret_cast<char*>(this+1);
}

BW_element* BW_element::join(BW_element *B)    // this will connect two circles
// Input:
// A0-A1-A2-A3-A4-A5...An-A0
// B0-B1-B2-B3-B4-B5...Bn-B0
// Output:
// A0-A1-A2-A3-A4-A5...An-B0-B1-B2-B3-B4-B5...Bn-A0

{
    assert(this != nullptr);
    assert(B != nullptr);

    int A_prev_offset = this->prev;
    int B_prev_offset = B->prev;

    A->prev = B_prev_offset;
    B->prev = A_prev_offset;
    
    BWE(B_prev_offset)->next = this->offset;
    BWE(A_prev_offset)->next = B->offset;
    return this;
}

BW_element* BW_element::add(BW_element *tag)
{
    if (this == nullptr) return this; // no where to add
    if (tag == nullptr) return this; // adding no elements

    BW_offset_t *list = (tag->flags & BIN_WRITE_ELEMENT_FLAG) ? &first_child : &first_attribute;
    if (*list == 0)
        *list = tag->offset; // direct link to new element
    else
       BWE(*list)->join(tag); // join elements
    return this;
}



BW2_plugin::BW2_plugin(const char *filename,Bin_xml_creator *bin_xml_creator,int max_pool_size)
    : Bin_src_plugin(filename,bin_xml_creator)
{
    this->max_pool_size = max_pool_size;
    this->pool = nullptr;
}
 
BW2_plugin::~BW2_plugin()
{
}


















































//-------------------------------------------------------------------------------------------------

BW_element_link::BW_element_link(BW_plugin *owner,BW_offset_t offset)
{
    assert(owner != nullptr);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= owner->pool_size);
    this->owner = owner;
    this->offset = offset;
}

BW_element_link  BW_element_link::add(BW_element_link child)
{
    assert(owner == child.owner); // mixing owners is not possible
    BW_element *container = BWE();
    BW_element *value = child.BWE();

    BW_offset_t *list;
    if (value->flags & BIN_WRITE_ELEMENT_FLAG)
        list = &container->first_child;
    else
        list = &container->first_attribute;

    if (*list == 0) // empty list
    {
        *list = child.offset; // assigning the 1st element
    }
    else
    {
        BW_element_link first(owner,*list);
        first.join(child);        
    }

    return *this; // returning container link
}

BW_element_link  BW_element_link::join(BW_element_link Blink)
{
// Must be the same BW_plugin
    assert(owner = Blink.owner);

// prepare all variables
    BW_element *A = BWE();
    BW_element *B = Blink.BWE();
    
    BW_offset_t A_prev_off = A->prev;
    BW_element *A_prev = BWE(A_prev_off);

    BW_offset_t B_prev_off = B->prev;
    BW_element *B_prev = BWE(B_prev_off);

// B_prev <---> A
    B_prev->next = offset;
    A->prev = B_prev_off;

// A_prev <---> B
    A_prev->next = Blink.offset;
    B->prev = A_prev_off;


// ...... B_prev <---> A ....... A_prev <---> B .......
    return *this; // returning container link
}

BW_element*      BW_element_link::BWE() const
{
    assert(owner != nullptr);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= owner->pool_size);
    return reinterpret_cast<BW_element *>(owner->pool + offset);
}

BW_element*      BW_element_link::BWE(BW_offset_t offset) const
{
    assert(owner != nullptr);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= owner->pool_size);
    return reinterpret_cast<BW_element *>(owner->pool + offset);
}

char*            BW_element_link::BWD() const
{
    assert(owner != nullptr);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= owner->pool_size);
    return owner->pool + offset + sizeof(BW_element);
}

char*            BW_element_link::BWD(BW_offset_t offset) const
{
    assert(owner != nullptr);
    assert(offset > 0);
    assert(offset+sizeof(BW_element) <= owner->pool_size);
    return owner->pool + offset + sizeof(BW_element);
}

//-------------------------------------------------------------------------------------------------

BW_element_link     BW_element_link::attrStr(int16_t id,const char *value)
{
    return *this;
}

BW_element_link     BW_element_link::attrHexStr(int16_t id,const char *value)
{
    return *this;
}

BW_element_link     BW_element_link::attrBLOB(int16_t id,const char *value,int32_t size)
{
    return *this;
}

BW_element_link     BW_element_link::attrInt32(int16_t id,int32_t value)
{
    assert(owner->getAttrType(id) == XBT_INT32 || owner->getAttrType(id) == XBT_VARIANT);
    BW_element_link adding   = owner->new_element(XBT_INT32,0); // only variable types gives size  --- sizeof(int32_t));
    BW_element      *element = adding.BWE();
    element->identification = id;
    element->flags          = 0;// attribute 
    element->value_type     = XBT_INT32;

    int32_t *dst            = reinterpret_cast<int32_t*>(element+1); // just after this element
    *dst                    = value; // store value
    add(adding);
    
    return *this;
}

BW_element_link     BW_element_link::attrInt64(int16_t id,int64_t value)
{
    return *this;
}

BW_element_link     BW_element_link::attrFloat(int16_t id,float value)
{
    return *this;
}

BW_element_link     BW_element_link::attrDouble(int16_t id,double value)
{
    return *this;
}

BW_element_link     BW_element_link::attrGUID(int16_t id,const char *value)
{
    return *this;
}

BW_element_link     BW_element_link::attrSHA1(int16_t id,const char *value)
{
    return *this;
}

BW_element_link     BW_element_link::attrTime(int16_t id,time_t value)
{
    return *this;
}

BW_element_link     BW_element_link::attrIPv4(int16_t id,const char *value)
{
    return *this;
}

BW_element_link     BW_element_link::attrIPv6(int16_t id,const char *value)
{
    return *this;
}

//-------------------------------------------------------------------------------------------------

BW_plugin::BW_plugin(int initial_pool_size,Bin_xml_creator *bin_xml_creator)
    :Bin_src_plugin("<internal-write>",bin_xml_creator)
{
// TODO: pool should be memory mapped file
    pool = new char[initial_pool_size];
    pool_size = initial_pool_size;
    memset(pool,0,pool_size);
    allocator = 0; 
    
    elements_offset = attributes_offset = -1;
    root = 0;
    elements = attributes = nullptr;
    max_element_id = max_attribute_id = -1;
}

BW_plugin::~BW_plugin()
{
    if (pool != nullptr)
        delete [] pool;
    pool = nullptr;

    if (elements != nullptr)
        delete [] elements;
    elements = nullptr;

    if (attributes != nullptr)
        delete [] attributes;
    attributes = nullptr;
}

BW_offset_t BW_plugin::allocate(int size)
{
// when not enough space - reallocate
    if (allocator + size > pool_size)
    { // this is slow, but should not occure very often
        char *new_pool = new char[pool_size * 2];
        memcpy(new_pool,pool,pool_size);
        memset(new_pool+pool_size,0,pool_size);
        delete [] pool;
        pool = new_pool;
        pool_size *= 2;
    }
    BW_offset_t offset = allocator;
    allocator += (size + 3) & ~3; // round up to 32 bits
    return offset;
}


BW_element_link  BW_plugin::new_element(XML_Binary_Type type,int size)
{
    switch(type)
    {
        case XBT_NULL:
            assert(size == 0);
            break;
        case XBT_VARIABLE:
            assert(type != XBT_VARIABLE); // must be known
            break;
        case XBT_STRING:
            size++; // terminating character
            break;
        case XBT_BINARY:
            size += sizeof(int32_t);
            break;
        case XBT_INT32:
            assert(size == 0);
            size = sizeof(int32_t);
            break;
        case XBT_INT64:
            assert(size == 0);
            size = sizeof(int64_t);
            break;
        case XBT_FLOAT:
            assert(size == 0);
            size = sizeof(float);
            break;
        case XBT_DOUBLE:
            assert(size == 0);
            size = sizeof(double);
            break;
        case XBT_HEX:
            size += sizeof(int32_t);
            break;
        case XBT_GUID:
            assert(size == 0);
            size = 16;
            break;
        case XBT_SHA1:
            assert(size == 0);
            size = 24;
            break;
        case XBT_UNIX_TIME:
            assert(size == 0);
            size = sizeof(int32_t);
            break;
        case XBT_IPv4:
            assert(size == 0);
            size = 4;
            break;
        case XBT_IPv6:
            assert(size == 0);
            size = 16;
            break;
        case XBT_VARIANT:
            assert(type != XBT_VARIANT);
            break;
    }
    BW_element_link result(this,allocate(sizeof(BW_element)+size));
    result.BWE()->value_type = type;
    switch(type)
    {
        case XBT_BINARY:
        case XBT_HEX:
        // size is stored to the 1st byte of data
            *reinterpret_cast<int32_t*>(1+result.BWE()) = size - sizeof(int32_t);
            break;
    }
    return result;
}

BW_element*   BW_plugin::BWE(BW_offset_t offset) const
{ 
    return reinterpret_cast<BW_element*>(pool + offset); 
}

void BW_plugin::registerElement(int16_t id,const char *name,XML_Binary_Type type)
// I want to fill symbol tables with element names    
// element names starts at offset elements_offset and ends at attributes_offset
{
    assert(name != nullptr);
// elements must be defined first
    if (elements_offset == -1)
    {
        elements_offset = allocator;
    }
    assert(attributes_offset == -1);
    assert(root == 0);

    MAXIMIZE(max_element_id,id);
    int len = strlen(name);

// allocation
    char *dst = pool + allocate(sizeof(int16_t)+sizeof(XML_Binary_Type_Stored)+len+1); // ID:word|Type:byte|string|NUL:byte
// id
    *reinterpret_cast<int16_t*>(dst) = id;
    dst += sizeof(int16_t);
// type
    *reinterpret_cast<XML_Binary_Type_Stored*>(dst) = type;
    dst += sizeof(XML_Binary_Type_Stored);
// name
    strcpy(dst,name);
}

void BW_plugin::registerAttribute(int16_t id,const char *name,XML_Binary_Type type)
// I want to fill symbol tables with attribute names    
{
    assert(name != nullptr);
// elements must be defined first
    assert(elements_offset != -1);
// root must be later
    assert(root == 0);

    if (attributes_offset == -1)
    {
        attributes_offset = allocator;
    }
    assert(root == 0);

    MAXIMIZE(max_attribute_id,id);
    int len = strlen(name);

// allocation
    char *dst = pool + allocate(sizeof(int16_t)+sizeof(XML_Binary_Type_Stored)+len+1); // ID:word|Type:byte|string|NUL:byte
// id
    *reinterpret_cast<int16_t*>(dst) = id;
    dst += sizeof(int16_t);
// type
    *reinterpret_cast<XML_Binary_Type_Stored*>(dst) = type;
    dst += sizeof(XML_Binary_Type_Stored);
// name
    strcpy(dst,name);
}

void BW_plugin::setRoot(const BW_element_link X)
{
// ::Initialize must be called
    assert(elements != nullptr);
//    assert(attributes != nullptr); -- this can be nullptr

    assert(root == 0);     // it is possible only once
    assert(X.owner == this);
    root = X.offset;

}


int32_t * BW_plugin::makeTable(int max_id,int32_t start,int32_t end)
{
    assert(max_id == -1 || start != -1);
    if (max_id < 0) return nullptr; // empty array
    
    int32_t *table = new int32_t[max_id+1];
    memset(table,0xff,sizeof(int32_t)*(max_id+1)); // 0xffffffff = -1

    while (start < end)
    {
        int id = *reinterpret_cast<int16_t*>(pool + start);
        assert(id >= 0 && id <= max_id);
        table[id] = start;
        start += 3 + strlen(pool+start+3)+1;
        ROUND32UP(start);
    }
    assert(start == end);
    return table;
}

XML_Binary_Type BW_plugin::getTagType(int16_t id) const
{
    if (id < 0 || id > max_element_id) return XBT_NULL;
    BW_offset_t offset = elements[id];
    return static_cast<XML_Binary_Type>(*reinterpret_cast<XML_Binary_Type_Stored*>(pool + offset + 2));  
}

XML_Binary_Type BW_plugin::getAttrType(int16_t id) const
{
    if (id < 0 || id > max_attribute_id) return XBT_NULL;
    BW_offset_t offset = attributes[id];
    return static_cast<XML_Binary_Type>(*reinterpret_cast<XML_Binary_Type_Stored*>(pool + offset + 2));  
}

//-------------------------------------------------------------------------------------------------

BW_element_link BW_plugin::tag(int16_t id)
{
    assert(getTagType(id) == XBT_NULL);
    BW_element_link result = this->new_element(XBT_NULL,0);
    BW_element      *element = result.BWE();

    element->init(id,XBT_NULL,BIN_WRITE_ELEMENT_FLAG,result.offset);

    return result;
}

BW_element_link BW_plugin::tagStr(int16_t id,const char *value)
{
    assert(getTagType(id) == XBT_STRING || getTagType(id) == XBT_VARIANT);
    BW_element_link result = this->new_element(XBT_STRING,strlen(value));
    BW_element      *element = result.BWE();
    
    element->init(id,XBT_STRING,BIN_WRITE_ELEMENT_FLAG,result.offset);

    strcpy(reinterpret_cast<char*>(element+1),value);
    return result;
}

BW_element_link BW_plugin::tagHexStr(int16_t id,const char *value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagBLOB(int16_t id,const char *value,int32_t size)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagInt32(int16_t id,int32_t value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagInt64(int16_t id,int64_t value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagFloat(int16_t id,float value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagDouble(int16_t id,double value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagGUID(int16_t id,const char *value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagSHA1(int16_t id,const char *value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagTime(int16_t id,time_t value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagIPv4(int16_t id,const char *value)
{
// TODO: not implemented
    assert(false);
}

BW_element_link BW_plugin::tagIPv6(int16_t id,const char *value)
{
// TODO: not implemented
    assert(false);
}

//-------------------------------------------------------------------------------------------------

bool BW_plugin::Initialize()
{
// It is possible to call this twice - it must be called before creating tags
    if (elements == nullptr)
        elements = makeTable(max_element_id,elements_offset,attributes_offset);
    if (attributes == nullptr)
        attributes = makeTable(max_attribute_id,attributes_offset,
            (root == 0 ? allocator : root));

    file_size = allocator; // must provide some info about size of file

    return true; // no inherited initialization required, Bin_src_plugin::Initialize would scan file size of "<internal-write>" with error code.
}

void *BW_plugin::getRoot()
{
    assert(root != 0);
    return pool + root;
}

const char *BW_plugin::getNodeName(void *element)
{
    if (element == nullptr) return "?";
    BW_element *E = reinterpret_cast<BW_element*>(element);
    assert(reinterpret_cast<char*>(E) - pool > 0);
    assert(pool + pool_size  - reinterpret_cast<char*>(E) >= sizeof(BW_element));

    if (E->flags & BIN_WRITE_ELEMENT_FLAG) // TAG
    {
        assert(elements != nullptr);
        switch (E->identification)
        {
            case XTNR_NULL: return "NULL";
            case XTNR_META_ROOT: return "meta_root";
            case XTNR_TAG_SYMBOLS: return "tag_symbols";
            case XTNR_PARAM_SYMBOLS: return "param_symbols";
            case XTNR_HASH_INDEX: return "hash_index";
            case XTNR_PAYLOAD: return "payload";
            default:
                assert(E->identification >= 0);
                assert(E->identification <= max_element_id);
                return pool + elements[E->identification] + 3; // +ID-2B,TYPE-1B
        }
    }
    else    // PARAM
    {
        assert(attributes != nullptr);
        switch (E->identification)
        {
            case XPNR_NULL : return "NULL";
            case XPNR_NAME : return "name";
            case XPNR_HASH : return "hash";
            case XPNR_MODIFIED : return "modified";
            case XPNR_COUNT : return "count";
            case XPNR_FORMAT_VERSION : return "version";
            case XPNR_TYPE_COUNT : return "type_count";

            default:
                assert(E->identification >= 0);
                assert(E->identification <= max_attribute_id);
                return pool + attributes[E->identification] + 3; // +ID-2B,TYPE-1B
        }
    }
}
// CHECKED----

const char *BW_plugin::getNodeValue(void *element)
{
    static char buffer[64];
    BW_element *E = reinterpret_cast<BW_element*>(element);
    switch (E->value_type)
    {
        case XBT_NULL: return nullptr; // empty value
        case XBT_STRING: return reinterpret_cast<char *>(element)+sizeof(BW_element); 
        case XBT_INT32: 
            sprintf(buffer,"%d",*reinterpret_cast<int32_t*>(reinterpret_cast<char *>(element)+sizeof(BW_element)));
            return buffer;
        case XBT_FLOAT:
            sprintf(buffer,"%f",*reinterpret_cast<float*>(reinterpret_cast<char *>(element)+sizeof(BW_element)));
            return buffer;
        default:
            assert(false);
            return nullptr;
    }
    
}

void BW_plugin::ForAllChildren(OnElement_t on_element,void *parent,void *userdata)
{
    BW_element *E = reinterpret_cast<BW_element*>(parent);
    if (E->first_child == 0) return; // no elements
    BW_element *child = BWE(E->first_child);
    for(;;)
    {
        on_element(child,userdata);
        if (child->next == E->first_child) break; // finished
        child = BWE(child->next);
    }
}

void BW_plugin::ForAllChildrenRecursively(OnElementRec_t on_element,void *parent,void *userdata,int deep)
{
    BW_element *E = reinterpret_cast<BW_element*>(parent);
    on_element(E,userdata,deep);
    if (E->first_child == 0) return; // no elements
    BW_element *child = BWE(E->first_child);
    for(;;)
    {
        ForAllChildrenRecursively(on_element,child,userdata,deep+1);

        if (child->next == E->first_child) break; // finished
        child = BWE(child->next);
    }
}

void BW_plugin::ForAllParams(OnParam_t on_param,void *element,void *userdata)
{
    BW_element *E = reinterpret_cast<BW_element*>(element);
    if (E->first_attribute == 0) return; // no attributes
    
    BW_element *child = BWE(E->first_attribute);
    for(;;)
    {
// typedef void (*OnParam_t)(const char *param_name,const char *param_value,void *element,void *userdata);
        
        on_param(getNodeName(child),getNodeValue(child),element,userdata);

        if (child->next == E->first_attribute) break; // finished
        child = BWE(child->next);
    }
}

}
