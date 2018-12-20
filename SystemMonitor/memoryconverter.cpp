#include "memoryconverter.h"
#include <cmath>
#include <assert.h>

memoryConverter::memoryConverter(double value, memoryUnit unit,
                                 unitStandard standard)
{
    memoryConverter::memoryEntry converted = fitMemoryValueToUnit(value, unit, getStandardKbSize(standard));
    this->memoryValue = converted.id;
    this->unit = converted.unit;
    this->standard = standard;
}

memoryConverter::memoryConverter(double value, memoryUnit unit, std::string standard) :
    memoryConverter(value, unit, memoryConverter::stringToStandard(standard))
{
    ;
}

memoryConverter::memoryConverter(const memoryConverter &other)
{
    this->memoryValue = other.getValue();
    this->unit = other.getUnit();
    this->standard = other.getStandard();
}

memoryConverter& memoryConverter::operator=(const memoryConverter& other)
{
    this->memoryValue = other.getValue();
    this->unit = other.getUnit();
    this->standard = other.getStandard();
    return *this;
}

bool memoryConverter::operator<(const memoryConverter &other) const
{
    memoryUnit otherUnit = other.unit;
    double otherValue = other.memoryValue;

    if (((int)unit) < ((int)otherUnit)) { return true; }
    if (((int)unit) > ((int)otherUnit)) { return false; }

    return memoryValue < otherValue;
}

double memoryConverter::getValue() const
{
    return memoryValue;
}

memoryUnit memoryConverter::getUnit() const
{
    return unit;
}

unitStandard memoryConverter::getStandard() const
{
    return standard;
}

int memoryConverter::getStandardKbSize(unitStandard standard)
{
    switch(standard) {
    case IEC:
    case JEDEC:
        return 1024;
    case SI:
        return 1000;
    }
    return 1;
}

std::string memoryConverter::getStandardUnit(unitStandard standard, memoryUnit unit)
{
    static std::string IEC_units[] = {
        "B", "KiB", "MiB", "GiB", "TiB"
    };

    static std::string JEDEC_units[] = {
        "B", "KB", "MB", "GB", "TB"
    };

    static std::string SI_units[] = {
        "B", "kB", "MB", "GB", "TB"
    };

    int pos = 0;
    for(; pos<memoryLookupLength; pos++) {
        if (memoryLookup[pos].unit == unit) {
            break;
        }
    }

    switch(standard) {
    case IEC:
        return IEC_units[pos];
        break;
    case JEDEC:
        return JEDEC_units[pos];
        break;
    case SI:
        return SI_units[pos];
        break;
    default:
        return "Error";
    }
}

memoryConverter::memoryEntry memoryConverter::fitMemoryValueToUnit(double memory, memoryUnit unit, int kb)
{
    if (memory < (1/kb)) {
        while(memory < (1/kb) && unit != memoryUnit::b) {
            memory *= kb;
            unit = prevMemoryUnit(unit);
        }
    }
    else
    {
        while(memory > kb) {
            memory /= kb;
            unit = nextMemoryUnit(unit);
        }
    }
    return {memory, unit};
}

memoryUnit memoryConverter::nextMemoryUnit(memoryUnit unit)
{
    for(int i = 0; i<memoryLookupLength; i++) {
        if (memoryLookup[i].unit == unit) {
            if (memoryLookup[i].id+1 > memoryLookupLength-1) {
                break;
            }
            return memoryLookup[i+1].unit;
        }
    }
    return unit;
}

memoryUnit memoryConverter::prevMemoryUnit(memoryUnit unit)
{
    for(int i = 0; i<memoryLookupLength; i++) {
        if (memoryLookup[i].unit == unit) {
            if (memoryLookup[i].id-1 < 0) {
                break;
            }
            return memoryLookup[i-1].unit;
        }
    }
    return unit;
}

void memoryConverter::convertTo(memoryUnit newUnit)
{
    int mulTimes = (int)this->unit - (int)newUnit;
    if (mulTimes == 0) {
        return;
    }

    if (mulTimes < 0) {
        this->memoryValue = this->memoryValue / (getStandardKbSize(standard) * abs(mulTimes));
    } else {
        this->memoryValue = this->memoryValue * (getStandardKbSize(standard) * mulTimes);
    }

    this->unit = newUnit;
}

std::string memoryConverter::to_string()
{
    std::string buffer;
    buffer += dbl2str(roundDouble(this->memoryValue, 1));
    buffer += " " + getStandardUnit(this->standard, this->unit);
    return buffer;
}

std::string memoryConverter::getUnitAsString()
{
    return getStandardUnit(this->standard, this->unit);
}

memoryConverter::operator std::string()
{
    return to_string();
}

double memoryConverter::truncateDouble(double input, unsigned int prec)
{
    return std::floor(std::pow(10, prec) * input) / std::pow(10, prec);
}

double memoryConverter::roundDouble(double input, unsigned int prec)
{
    return std::roundf(std::pow(10, prec) * input) / std::pow(10, prec);
}

std::string memoryConverter::dbl2str(double d)
{
    size_t len = std::snprintf(0, 0, "%.10f", d);
    std::string s(len+1, 0);
    std::snprintf(&s[0], len+1, "%.10f", d);
    s.pop_back();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if(s.back() == '.') {
        s.pop_back();
    }
    return s;
}
