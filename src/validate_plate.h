#include <string.h>

bool is_num(char c){
    return (c >= '0' && c <= '9') ? true : false;
}

bool is_letter(char c){
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? true : false;
}

/*
brasil: ABC1D23, or ABC12D3
argentina: AB 123 CD
paraguai: ABCD 123 or 123 ABCD
uruguai: ABC 1234
venezuela: AB 123 CD
bolivia: AB 12345
*/

bool validate_argentina(char *plate){
    if(strlen(plate) > 9) return false;

    if (is_letter(plate[0])                         && 
        is_letter(plate[1])                         &&
        plate[2] == ' '                             &&
        is_num(plate[3])                            &&
        is_num(plate[4])                            &&
        is_num(plate[5])                            &&
        plate[6] == ' '                             &&
        is_letter(plate[7])                         &&
        is_letter(plate[8])) 
                            return true;
    else return false;
}
bool validate_brazil(char *plate){
    if(strlen(plate) > 7) return false;

    if (is_letter(plate[0])                         && 
        is_letter(plate[1])                         &&
        is_letter(plate[2])                         &&
        is_num(plate[3])                            &&
        (is_letter(plate[4]) || is_num(plate[4]))   &&
        is_num(plate[5])                            &&
        is_num(plate[6])) 
                        return true;
    else return false;
}

bool validate_paraguay(char *plate){
    if(strlen(plate) > 8) return false;

    if ((is_letter(plate[0]) || is_num(plate[0]))       && 
        (is_letter(plate[1]) || is_num(plate[1]))       &&
        (is_letter(plate[2]) || is_num(plate[2]))       &&
        (is_letter(plate[3]) || plate[3] == ' ')        &&
        (plate[4] == ' '     || is_letter(plate[4]))    &&
        (is_letter(plate[5]) || is_num(plate[5]))       &&
        (is_letter(plate[6]) || is_num(plate[6]))       &&
        (is_letter(plate[7]) || is_num(plate[7]))) 
                                                    return true;

    else return false;
}
//uruguai: ABC 1234

bool validate_uruguai(char *plate){
    if(strlen(plate) > 8) return false;

    if (is_letter(plate[0]) && 
        is_letter(plate[1]) &&
        is_letter(plate[2]) &&
        plate[3] == ' '     &&
        is_num(plate[4])    &&
        is_num(plate[5])    &&
        is_num(plate[6])    &&
        is_num(plate[7])) 
                        return true;
    else return false;

}

bool validate_venezuela(char *plate){
    return validate_argentina(plate);
}

bool validate_bolivia(char *plate){
    if(strlen(plate) > 8) return false;

    if (is_letter(plate[0]) && 
        is_letter(plate[1]) &&
        plate[2] == ' '     &&
        is_num(plate[3])    &&
        is_num(plate[4])    &&    
        is_num(plate[5])    &&
        is_num(plate[6])    &&
        is_num(plate[7])
    ) return true;

    else return false;

}
bool validate_mercosul_plate(char *plate){
    if (
        validate_argentina(plate)   ||
        validate_brazil(plate)      ||
        validate_paraguay(plate)    ||
        validate_venezuela(plate)   ||
        validate_uruguai(plate)     ||
        validate_paraguay(plate)    ||
        validate_paraguay(plate)
    ) return true;

    else return false;
}