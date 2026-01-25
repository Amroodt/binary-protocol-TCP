# binary-protocol-TCP

[uint32_t length] [uint8_t type][payload]

***receive***

#### Ping Request

| Field      | Type       | Description                       | Size    | Content |
|------------|------------|-----------------------------------|---------|---------|
| length     | uint32_t   | Number of bytes in type + payload | 8 bytes | 0x01    |
| type       | uint8_t    | Message type Identifier           | 4 bytes | 0x01    |
| payload    | bytes [N]  | Message-specific payload          | 0 bytes | NULL    |

Frame Length = 5

#### Ping Response
| Field      | Type       | Description                       | Size    | Content |
|------------|------------|-----------------------------------|---------|---------|
| length     | uint32_t   | Number of bytes in type + payload | 8 bytes | 0x01    |
| type       | uint8_t    | Message type Identifier           | 4 bytes | 0x02    |
| payload    | bytes [N]  | Message-specific payload          | 0 bytes | NULL    |

Frame Length = 5


``` 
type:   0x01 -> PingRequest 
            length_field = 4 bytes (counts the bytes after the len field)
            type_field = 1 byte
            payload = NULL
            frame_size = 5
        ---------------->
        0x02: PingResponse
            length_field = 4 bytes
            type_field = 1 byte
            payload = NULL
            frame_size =5
        
```





enum class MsgType : uint8_t {

    PingReq  = 0x01,
    PingResp = 0x02,

    EchoReq  = 0x03,
    EchoResp = 0x04,

    SetReq   = 0x10,
    SetResp  = 0x11,

    GetReq   = 0x12,
    GetResp  = 0x13,

    DelReq   = 0x14,
    DelResp  = 0x15,

    ErrResp  = 0x7F
};