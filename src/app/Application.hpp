#pragma once

namespace viibe
{
    class Application
    {
    public:
        bool Initialize();
        int Run();
        void Shutdown();

    private:
        bool m_initialized{ false };
    };
}
