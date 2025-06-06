#pragma once

#include "yql_user_data.h"
#include <yql/essentials/providers/common/proto/udf_resolver.pb.h>

#include <yql/essentials/core/yql_holding_file_storage.h>
#include <yql/essentials/public/issue/yql_issue.h>
#include <yql/essentials/public/udf/udf_log.h>

#include <util/generic/maybe.h>
#include <util/generic/vector.h>

namespace NYql {

class TTypeAnnotationNode;
struct TUserDataBlock;
struct TExprContext;

struct TFilePathWithMd5 {
    TString Path;
    TString Md5;

    explicit TFilePathWithMd5(const TString& path = "", const TString& md5 = "")
        : Path(path)
        , Md5(md5)
    {
    }

    TFilePathWithMd5& operator=(const TFilePathWithMd5& other) = default;
};

class IUdfResolver : public TThrRefBase {
public:
    using TPtr = TIntrusiveConstPtr<IUdfResolver>;

    virtual ~IUdfResolver() = default;

    struct TFunction {
        // input
        TPosition Pos;
        TString Name;
        TString TypeConfig;
        const TTypeAnnotationNode* UserType = nullptr;
        THashMap<TString, TString> SecureParams;

        // output
        TString NormalizedName;
        const TTypeAnnotationNode* NormalizedUserType = nullptr;
        const TTypeAnnotationNode* RunConfigType = nullptr;
        const TTypeAnnotationNode* CallableType = nullptr;
        bool SupportsBlocks = false;
        bool IsStrict = false;
        TVector<TString> Messages;
    };

    struct TImport {
        // input
        TPosition Pos;
        TString FileAlias;
        const TUserDataBlock* Block = nullptr;

        // output
        TMaybe<TVector<TString>> Modules;
    };

    /*
    Returns nothing if module is not a system one
    Always returns frozen path
    */
    virtual TMaybe<TFilePathWithMd5> GetSystemModulePath(const TStringBuf& moduleName) const = 0;
    virtual bool LoadMetadata(const TVector<TImport*>& imports,
        const TVector<TFunction*>& functions, TExprContext& ctx, NUdf::ELogLevel logLevel, THoldingFileStorage& storage) const = 0;

    virtual TResolveResult LoadRichMetadata(const TVector<TImport>& imports, NUdf::ELogLevel logLevel, THoldingFileStorage& storage) const = 0;
    virtual bool ContainsModule(const TStringBuf& moduleName) const = 0;
};

TResolveResult LoadRichMetadata(const IUdfResolver& resolver, const TVector<TUserDataBlock>& blocks, THoldingFileStorage& storage, NUdf::ELogLevel logLevel = NUdf::ELogLevel::Info);
TResolveResult LoadRichMetadata(const IUdfResolver& resolver, const TVector<TString>& paths, THoldingFileStorage& storage, NUdf::ELogLevel logLevel = NUdf::ELogLevel::Info);

}
