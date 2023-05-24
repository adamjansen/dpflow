#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include "dplib/core/Application.h"

#include "dplib/core/Platform.h"

using namespace datapanel::core;

Application::Application() : running(false), started(false)
{
    m_logger = spdlog::stdout_color_st("Application");
    m_logger->info("Creating application");
    platform = createPlatform();
    platform->appStart();
}

Application::~Application()
{
    if (platform != nullptr)
        platform->appStop();
    m_logger->flush();
    m_logger.reset();
}

int Application::run()
{
    if (started) {
        m_logger->critical("Application already running");
        return -1;
    }
    m_logger->info("Starting application");
    started = true;
    running = true;
    while (running) { platform->processEvents(); }

    m_logger->info("Exited with status {}", exitStatus);

    return exitStatus;
}

void Application::exit(int status)
{
    m_logger->info("Exiting application, status={}", status);
    exitStatus = status;
    running = false;
}
