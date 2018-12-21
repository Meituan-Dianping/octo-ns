##use this to build mns idl files
#!/bin/sh
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-common/src/main/thrift/naming_common.thrift
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-common/src/main/thrift/naming_data.thrift 
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-common/src/main/thrift/naming_service.thrift 
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-common/src/main/thrift/unified_protocol.thrift
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-mnsc/src/main/thrift/mnsc_service.thrift
    ./thrift-0.8.0_build/bin/thrift --gen cpp -o ../ ../../common/idl-mns/idl-mnsc/src/main/thrift/mnsc_data.thrift
