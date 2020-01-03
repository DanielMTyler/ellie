/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include <cstdarg>
#include <cstdio>

class Log : public ILog
{
private:
    enum class Type
    {
        fatal,
        warn,
        info,
        debug,
        trace
    };

    FILE *file = nullptr;
    static const uint32 bufferSize = KIBIBYTES(2);
    char buffer[bufferSize];

    void log(const char *system, const char *format, Type type, va_list args)
    {
        if (!file)
            return;

        // Ignore vsnprintf errors.
        if (vsnprintf(buffer, bufferSize, format, args))
        {
            // @todo Add a timestamp to logging operations.
            // Ignore fprintf errors.
            switch (type)
            {
                case Type::fatal:
                    fprintf(file, "[%s] FATAL: %s\r\n", system, buffer);
                    break;
                case Type::warn:
                    fprintf(file, "[%s] WARN: %s\r\n", system, buffer);
                    break;
                case Type::info:
                    fprintf(file, "[%s]: %s\r\n", system, buffer);
                    break;
                case Type::debug:
                    fprintf(file, "[%s] DEBUG: %s\r\n", system, buffer);
                    break;
                case Type::trace:
                    fprintf(file, "[%s] TRACE: %s\r\n", system, buffer);
                    break;
                default:
                    INVALID_CODE_PATH;
                    break;
            }

            // Flush in case of crashes.
            fflush(file);
        }
    }

public:
    virtual ~Log() override
    {
        if (file)
        {
            fclose(file);
            file = nullptr;
        }
    }

    bool init(const char *filename)
    {
        ASSERT(!file);
        ASSERT(filename);

        file = fopen(filename, "wb");
        if (file)
            return true;
        else
            return false;
    }

    virtual void fatal(const char *system, const char *format, ...) override
    {
        va_list args;
        va_start(args, format);
        log(system, format, Type::fatal, args);
        va_end(args);
    }

    virtual void warn(const char *system, const char *format, ...) override
    {
        va_list args;
        va_start(args, format);
        log(system, format, Type::warn, args);
        va_end(args);
    }

    virtual void info(const char *system, const char *format, ...) override
    {
        va_list args;
        va_start(args, format);
        log(system, format, Type::info, args);
        va_end(args);
    }

    virtual void debug(const char *system, const char *format, ...) override
    {
        va_list args;
        va_start(args, format);
        log(system, format, Type::debug, args);
        va_end(args);
    }

    virtual void trace(const char *system, const char *format, ...) override
    {
        va_list args;
        va_start(args, format);
        log(system, format, Type::trace, args);
        va_end(args);
    }
};
