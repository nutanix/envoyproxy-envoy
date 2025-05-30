#pragma once

#include "envoy/extensions/common/aws/v3/credential_provider.pb.h"

#include "source/common/common/logger.h"
#include "source/extensions/common/aws/region_provider.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Aws {

/**
 * Retrieve AWS region name from the environment
 */
class EnvironmentRegionProvider : public RegionProvider, public Logger::Loggable<Logger::Id::aws> {
public:
  EnvironmentRegionProvider() = default;

  absl::optional<std::string> getRegion() override;

  absl::optional<std::string> getRegionSet() override;
};

class AWSCredentialsFileRegionProvider : public RegionProvider,
                                         public Logger::Loggable<Logger::Id::aws> {
public:
  AWSCredentialsFileRegionProvider(
      const envoy::extensions::common::aws::v3::CredentialsFileCredentialProvider&
          credential_file_config);

  absl::optional<std::string> getRegion() override;

  absl::optional<std::string> getRegionSet() override;

private:
  absl::optional<std::string> credential_file_path_;
  absl::optional<std::string> profile_;
};

class AWSConfigFileRegionProvider : public RegionProvider,
                                    public Logger::Loggable<Logger::Id::aws> {
public:
  AWSConfigFileRegionProvider() = default;

  absl::optional<std::string> getRegion() override;

  absl::optional<std::string> getRegionSet() override;
};

class RegionProviderChainFactories {
public:
  virtual ~RegionProviderChainFactories() = default;

  virtual RegionProviderSharedPtr createEnvironmentRegionProvider() const PURE;
  virtual RegionProviderSharedPtr createAWSCredentialsFileRegionProvider(
      const envoy::extensions::common::aws::v3::CredentialsFileCredentialProvider&
          credential_file_config) const PURE;
  virtual RegionProviderSharedPtr createAWSConfigFileRegionProvider() const PURE;
};

/**
 * AWS region provider chain, supporting environment, envoy configuration, AWS config and AWS
 * profile.
 */
class RegionProviderChain : public RegionProvider,
                            public RegionProviderChainFactories,
                            public Logger::Loggable<Logger::Id::aws> {
public:
  RegionProviderChain(const envoy::extensions::common::aws::v3::CredentialsFileCredentialProvider&
                          credential_file_config = {});

  ~RegionProviderChain() override = default;

  void add(const RegionProviderSharedPtr& region_provider) {
    providers_.emplace_back(region_provider);
  }

  absl::optional<std::string> getRegion() override;

  absl::optional<std::string> getRegionSet() override;

  RegionProviderSharedPtr createEnvironmentRegionProvider() const override {
    return std::make_shared<EnvironmentRegionProvider>();
  }
  RegionProviderSharedPtr createAWSCredentialsFileRegionProvider(
      const envoy::extensions::common::aws::v3::CredentialsFileCredentialProvider&
          credential_file_config) const override {
    return std::make_shared<AWSCredentialsFileRegionProvider>(credential_file_config);
  }
  RegionProviderSharedPtr createAWSConfigFileRegionProvider() const override {
    return std::make_shared<AWSConfigFileRegionProvider>();
  }

protected:
  std::list<RegionProviderSharedPtr> providers_;
};

using EnvironmentRegionProviderPtr = std::shared_ptr<EnvironmentRegionProvider>;
using AWSCredentialsFileRegionProviderPtr = std::shared_ptr<AWSCredentialsFileRegionProvider>;
using AWSConfigFileRegionProviderPtr = std::shared_ptr<AWSConfigFileRegionProvider>;

} // namespace Aws
} // namespace Common
} // namespace Extensions
} // namespace Envoy
