#include "IOpenCVEngine.h"
#include "CommonPackageManager.h"
#include "HardwareDetector.h"
#include <utils/Log.h>
#include <algorithm>
#include <stdio.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "CommonPackageManager"

using namespace std;

set<string> CommonPackageManager::GetInstalledVersions()
{
    set<string> result;
    vector<PackageInfo> installed_packages = GetInstalledPackages();
    
    for (vector<PackageInfo>::const_iterator it = installed_packages.begin(); it != installed_packages.end(); ++it)
    {
	string version = it->GetVersion();
	assert(!version.empty());
	result.insert(version);
    }
    
    return result;
}

bool CommonPackageManager::CheckVersionInstalled(const std::string& version, int platform, int cpu_id)
{
    bool result = false;
    LOGD("CommonPackageManager::CheckVersionInstalled() begin");
    PackageInfo target_package(version, platform, cpu_id);
    LOGD("GetInstalledPackages() call");
    vector<PackageInfo> packages = GetInstalledPackages();
    
    for (vector<PackageInfo>::const_iterator it = packages.begin(); it != packages.end(); ++it)
    {
	LOGD("Found package: \"%s\"", it->GetFullName().c_str());
    }
    
    if (!packages.empty())
    {
	result = (packages.end() != find(packages.begin(), packages.end(), target_package));
    }
    LOGD("CommonPackageManager::CheckVersionInstalled() end");
    return result;
}

bool CommonPackageManager::InstallVersion(const std::string& version, int platform, int cpu_id)
{
    LOGD("CommonPackageManager::InstallVersion() begin");
    PackageInfo package(version, platform, cpu_id);    
    return InstallPackage(package);
}

string CommonPackageManager::GetPackagePathByVersion(const std::string& version, int platform, int cpu_id)
{
    string result;
    PackageInfo target_package(version, platform, cpu_id);
    vector<PackageInfo> packages = GetInstalledPackages();
    
    if (!packages.empty())
    {
	vector<PackageInfo>::iterator it = find(packages.begin(), packages.end(), target_package);
	if (packages.end() != it)
	{
	    result = it->GetInstalationPath();
	}
    }
    
    return result;
}

CommonPackageManager::~CommonPackageManager()
{
}
