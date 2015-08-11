#ifndef IBFADATATYPE_H
#define IBFADATATYPE_H

enum FaDataType { GROUPS=1, PROFILES, ALIASES } ;

inline const char* faDataTypeStr ( FaDataType pFaDataType )
{
    switch (pFaDataType) {
        case GROUPS:
            return "GROUPS" ;
            break ;
        case PROFILES:
            return "PROFILES" ;
            break ;
        case ALIASES:
            return "ALIASES" ;
            break ;
    }
    return 0 ;
}

#endif // IBFADATATYPE_H

