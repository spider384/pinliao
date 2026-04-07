//==============================================================================
//  NX2306 拼料功能 - 头文件 (最终可用版)
//==============================================================================
#ifndef UI2_H_INCLUDED
#define UI2_H_INCLUDED

#include <uf_defs.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_Group.hxx>
#include <NXOpen/BlockStyler_Button.hxx>
#include <NXOpen/BlockStyler_FaceCollector.hxx>
#include <NXOpen/BlockStyler_SpecifyPoint.hxx>
#include <NXOpen/BlockStyler_Toggle.hxx>
#include <NXOpen/BlockStyler_LinearDimension.hxx>
#include <NXOpen/BlockStyler_SelectObject.hxx>
#include <NXOpen/BlockStyler_Tree.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Face.hxx>
#include <NXOpen/TaggedObject.hxx>
#include <NXOpen/DisplayManager.hxx>
#include <NXOpen/Update.hxx>
#include <NXOpen/DisplayableObject.hxx>

// 使用别名解决命名冲突
using NXOpen::BlockStyler::Group;
using NXOpen::BlockStyler::SelectObject;

class UI2
{
public:
    static NXOpen::Session* theSession;
    static NXOpen::UI* theUI;

    UI2();
    ~UI2();
    NXOpen::BlockStyler::BlockDialog::DialogResponse Launch();

    void initialize_cb();
    void dialogShown_cb();
    int apply_cb();
    int ok_cb();
    int update_cb(NXOpen::BlockStyler::UIBlock* block);
    NXOpen::BlockStyler::PropertyList* GetBlockProperties(const char* blockID);

private:
    // 零件信息结构体
    struct PartInfo
    {
        NXOpen::Body* body;
        NXOpen::Face* referenceFace;
        int index;
    };

    // 核心功能函数
    void HideSelectedBodies();
    void ShowAndArrangePartsAtPoint();
    void GetAllSelectedBodies();
    void SetBodyVisibility(NXOpen::Body* body, bool visible);
    void CreateStockMessage(int partCount, double totalWidth, double totalDepth, double maxHeight,
        double baseX, double baseY, double baseZ);

    // 辅助函数
    void ShowMessage(const std::string& title, NXOpen::NXMessageBox::DialogType type, const std::string& message);
    double GetDoubleValue(NXOpen::BlockStyler::UIBlock* block);
    bool GetToggleValue(NXOpen::BlockStyler::Toggle* toggle);
    void GetPointCoordinates(double& x, double& y, double& z);

    const char* theDlxFileName;
    NXOpen::BlockStyler::BlockDialog* theDialog;

    // UI控件 - 使用原始文件中的 block id
    NXOpen::BlockStyler::Group* ui_group1;
    NXOpen::BlockStyler::Group* ui_group2;
    NXOpen::BlockStyler::Group* ui_group3;

    NXOpen::BlockStyler::Button* ui_button0;
    NXOpen::BlockStyler::Button* ui_button1;
    NXOpen::BlockStyler::FaceCollector* ui_face_select0;
    NXOpen::BlockStyler::SpecifyPoint* ui_point0;
    NXOpen::BlockStyler::Toggle* ui_toggle0;
    NXOpen::BlockStyler::Toggle* ui_toggle01;
    NXOpen::BlockStyler::Toggle* ui_toggle02;

    NXOpen::BlockStyler::LinearDimension* ui_linear_dim0;
    NXOpen::BlockStyler::LinearDimension* ui_linear_dim01;
    NXOpen::BlockStyler::LinearDimension* ui_linear_dim02;

    NXOpen::BlockStyler::SelectObject* ui_selection0;
    NXOpen::BlockStyler::Tree* ui_tree_control01;

    // 数据存储
    std::vector<PartInfo> m_selectedParts;

    // 间距参数
    double m_partSpacing;
    double m_outerMargin;
    double m_topSpacing;

    bool m_generateStock;
};

#endif