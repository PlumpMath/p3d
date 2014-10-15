#ifndef IMATERIALSINFO_H
#define IMATERIALSINFO_H

class IMaterialsInfo
{
public:
    virtual void setMaterialProperty(int materialIndex, const char* property, const char* value) = 0;
};

#endif // IMATERIALSINFO_H
