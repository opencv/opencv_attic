#ifndef __COMMON_PACKAGE_MANAGER_H__
#define __COMMON_PACKAGE_MANAGER_H__

#include "IPackageManager.h"
#include "PackageInfo.h"
#include <set>
#include <vector>
#include <string>

class CommonPackageManager: public IPackageManager
{
public:
    std::set<std::string> GetInstalledVersions();
    bool CheckVersionInstalled(const std::string& version, int platform, int cpu_id);
    bool InstallVersion(const std::string& version, int platform, int cpu_id);
    std::string GetPackagePathByVersion(const std::string& version, int platform, int cpu_id);
    virtual ~CommonPackageManager();

protected:
    virtual bool InstallPackage(const PackageInfo& package) = 0;
    virtual std::vector<PackageInfo> GetInstalledPackages() = 0;   
};


#endif