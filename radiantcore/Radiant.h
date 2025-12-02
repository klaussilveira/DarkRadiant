#pragma once

#include "iradiant.h"
#include "modulesystem/ModuleRegistry.h"

namespace applog { class LogFile; }
namespace language { class LanguageManager; }

namespace radiant
{

class MessageBus;

class Radiant: public IRadiant
{
	IApplicationContext& _context;
	std::unique_ptr<applog::LogFile> _logFile;
	std::unique_ptr<module::ModuleRegistry> _moduleRegistry;
	std::unique_ptr<MessageBus> _messageBus;
	std::unique_ptr<language::LanguageManager> _languageManager;

public:
	Radiant(IApplicationContext& context);

	~Radiant();

	std::string getName() const override;

	applog::ILogWriter& getLogWriter() override;
	module::ModuleRegistry& getModuleRegistry() override;
	radiant::IMessageBus& getMessageBus() override;
	language::ILanguageManager& getLanguageManager() override;
	void startup() override;

	static std::shared_ptr<Radiant>& InstancePtr();

private:
	void createLogFile();
};

}