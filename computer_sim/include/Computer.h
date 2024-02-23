#ifndef _COMPUTER
#define _COMPUTER
#include <string>

class CUDA
{
public:
    CUDA(int);
    const int get_numCores() const;
    std::string render() const;
    std::string trainModel() const;

private:
    const int numCores;
};

class ALU
{
public:
    ALU(int);
    const int get_numPerCores() const;
    int add(int, int) const;
    int subtract(int, int) const;
    int multiply(int, int) const;

private:
    const int numPerCores;
};

class GPU
{
public:
    GPU(int);
    const CUDA &get_cuda() const;
    std::string execute(std::string) const;

private:
    const CUDA cuda;
};

class CPU
{
public:
    CPU(int);
    const ALU &get_alu() const;
    int execute(std::string) const;

private:
    const ALU alu;
};

class Computer
{
public:
    Computer();
    void set_attachedCPU(const CPU &);
    void set_attachedGPU(const GPU &);
    void operator+(const CPU &);
    void operator+(const GPU &);
    void execute(std::string) const;
    const CPU *get_attachedCPU() const;
    const GPU *get_attachedGPU() const;

private:
    const CPU *attachedCPU{NULL};
    const GPU *attachedGPU{NULL};
};

#endif