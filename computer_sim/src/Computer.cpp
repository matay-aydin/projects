#include "Computer.h"
#include <iostream>

Computer::Computer()
{
    std::cout << "Computer is ready" << std::endl;
}

void Computer::set_attachedCPU(const CPU &newCpuAddress)
{
    this->attachedCPU = &newCpuAddress;
}

void Computer::set_attachedGPU(const GPU &newGpuAddress)
{
    this->attachedGPU = &newGpuAddress;
}

void Computer::operator+(const CPU &attachment)
{
    if (this->attachedCPU != NULL)
    {
        std::cout << "There is already a CPU" << std::endl;
        return;
    }
    this->attachedCPU = &attachment;
    std::cout << "CPU is attached" << std::endl;
}

void Computer::operator+(const GPU &attachment)
{
    if (this->attachedGPU != NULL)
    {
        std::cout << "There is already a GPU" << std::endl;
        return;
    }
    this->attachedGPU = &attachment;
    std::cout << "GPU is attached" << std::endl;
}

void Computer::execute(std::string op) const
{
    if (op == "add" || op == "subtract" || op == "multiply")
    {
        int result = this->attachedCPU->execute(op);
        std::cout << result << std::endl;
    }
    else if (op == "render" || op == "trainModel")
    {
        std::string result = this->attachedGPU->execute(op);
        std::cout << result << std::endl;
    }
    else
    {
        std::cout << "Not a valid opcode!" << std::endl;
    }
}

const CPU *Computer::get_attachedCPU() const
{
    return this->attachedCPU;
}

const GPU *Computer::get_attachedGPU() const
{
    return this->attachedGPU;
}

CPU::CPU(int alus) : alu{alus}
{
    std::cout << "CPU is ready" << std::endl;
}

const ALU &CPU::get_alu() const
{
    return this->alu;
}

int CPU::execute(std::string opcode) const
{
    std::cout << "Enter two integers:" << std::endl;
    int operand1, operand2, result = 0;
    std::cin >> operand1 >> operand2;
    if (opcode == "add")
        result = this->alu.add(operand1, operand2);
    else if (opcode == "subtract")
        result = this->alu.subtract(operand1, operand2);
    else if (opcode == "multiply")
        result = this->alu.multiply(operand1, operand2);
    else
        std::cout << "Not a valid opcode!" << std::endl;
    return result;
}

GPU::GPU(int cudas) : cuda{cudas}
{
    std::cout << "GPU is ready" << std::endl;
}

const CUDA &GPU::get_cuda() const
{
    return this->cuda;
}

std::string GPU::execute(std::string opcode) const
{
    std::string result = "";
    if (opcode == "render")
        result = this->cuda.render();
    else if (opcode == "trainModel")
        result = this->cuda.trainModel();
    else
        std::cout << "Not a valid opcode!" << std::endl;
    return result;
}

ALU::ALU(int perCores) : numPerCores{perCores}
{
    std::cout << "ALU is ready" << std::endl;
}

const int ALU::get_numPerCores() const
{
    return numPerCores;
}

int ALU::add(int num1, int num2) const
{
    return num1 + num2;
}

int ALU::subtract(int num1, int num2) const
{
    return num1 - num2;
}

int ALU::multiply(int num1, int num2) const
{
    return num1 * num2;
}

CUDA::CUDA(int cores) : numCores{cores}
{
    std::cout << "CUDA is ready" << std::endl;
}

const int CUDA::get_numCores() const
{
    return this->numCores;
}

std::string CUDA::render() const
{
    std::string result = "Video is rendered";
    return result;
}

std::string CUDA::trainModel() const
{
    std::string result = "AI Model is trained";
    return result;
}