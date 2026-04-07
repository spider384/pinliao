//==============================================================================
//  WARNING!!  This file is overwritten by the Block Styler while generating
//  automation code. Any modifications to this file will be lost after
//  generating the code again.
//==============================================================================
#ifndef UI2_H_INCLUDED
#define UI2_H_INCLUDED

#include <uf_defs.h>
#include <uf_ui_types.h>
#include <iostream>
#include <vector>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_Node.hxx>
#include <NXOpen/BlockStyler_Group.hxx>
#include <NXOpen/BlockStyler_Button.hxx>
#include <NXOpen/BlockStyler_FaceCollector.hxx>
#include <NXOpen/BlockStyler_SpecifyPoint.hxx>
#include <NXOpen/BlockStyler_Toggle.hxx>
#include <NXOpen/BlockStyler_LinearDimension.hxx>
#include <NXOpen/BlockStyler_StringBlock.hxx>
#include <NXOpen/BlockStyler_SelectObject.hxx>
#include <NXOpen/BlockStyler_Tree.hxx>
#include <NXOpen/Body.hxx>
#include <NXOpen/Point.hxx>

using namespace std;
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

class DllExport UI2
{
public:
    static Session* theSession;
    static UI* theUI;
    UI2();
    ~UI2();
    NXOpen::BlockStyler::BlockDialog::DialogResponse Launch();

    void initialize_cb();
    void dialogShown_cb();
    int apply_cb();
    int ok_cb();
    int update_cb(NXOpen::BlockStyler::UIBlock* block);
    PropertyList* GetBlockProperties(const char* blockID);

    // 拼料功能函数
    void HideSelectedBodies();
    void InsertAndMergeParts();
    double GetBodyWidth(Body* body);
    double GetBodyHeight(Body* body);
    Body* CopyBody(Part* part, Body* originalBody);
    void MoveBody(Body* body, Point3d targetPos);
    void MergeBodies(Part* part, vector<Body*>& bodies);
    void CreateStockBody(Part* part, vector<Body*>& parts);

private:
    const char* theDlxFileName;
    NXOpen::BlockStyler::BlockDialog* theDialog;

    // UI控件对象
    Group* group1;
    Button* button0;
    Button* button01;
    Group* group0;
    FaceCollector* face_select0;
    SpecifyPoint* point0;
    Toggle* toggle0;
    Group* group;
    LinearDimension* linear_dim0;
    LinearDimension* linear_dim01;
    LinearDimension* linear_dim02;
    Toggle* toggle01;
    Toggle* toggle02;
    StringBlock* string0;
    StringBlock* string01;
    SelectObject* selection0;
    Tree* tree_control01;
};

#endif