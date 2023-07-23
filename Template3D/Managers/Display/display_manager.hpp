#pragma once

class SDisplayManager
{
public:
	SDisplayManager(SDisplayManager&) = delete;
	static SDisplayManager& get();

	void startup();

	void shutdown();

private:
	SDisplayManager() = default;
	~SDisplayManager() = default;
};

