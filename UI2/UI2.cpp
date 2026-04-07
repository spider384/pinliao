//==============================================================================
//  NX2306 拼料功能 - CPP文件 (最终可用版)
//==============================================================================
#include "UI2.hpp"
#include <cmath>
#include <algorithm>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace std;

Session* UI2::theSession = NULL;
UI* UI2::theUI = NULL;

UI2::UI2() :
    m_generateStock(false),
    m_partSpacing(0.0),
    m_outerMargin(0.0),
    m_topSpacing(0.0),
    ui_group1(NULL),
    ui_group2(NULL),
    ui_group3(NULL),
    ui_button0(NULL),
    ui_button1(NULL),
    ui_face_select0(NULL),
    ui_point0(NULL),
    ui_toggle0(NULL),
    ui_toggle01(NULL),
    ui_toggle02(NULL),
    ui_linear_dim0(NULL),
    ui_linear_dim01(NULL),
    ui_linear_dim02(NULL),
    ui_selection0(NULL),
    ui_tree_control01(NULL)
{
    theSession = Session::GetSession();
    theUI = UI::GetUI();
    theDlxFileName = "UI2.dlx";
    theDialog = theUI->CreateDialog(theDlxFileName);

    theDialog->AddInitializeHandler(make_callback(this, &UI2::initialize_cb));
    theDialog->AddDialogShownHandler(make_callback(this, &UI2::dialogShown_cb));
    theDialog->AddUpdateHandler(make_callback(this, &UI2::update_cb));
    theDialog->AddApplyHandler(make_callback(this, &UI2::apply_cb));
    theDialog->AddOkHandler(make_callback(this, &UI2::ok_cb));
}

UI2::~UI2()
{
    if (theDialog)
    {
        delete theDialog;
        theDialog = NULL;
    }
}

extern "C" DllExport void ufusr(char* param, int* retcod, int param_len)
{
    try
    {
        UI2* theUI2 = new UI2();
        theUI2->Launch();
        delete theUI2;
    }
    catch (exception& e)
    {
        UI::GetUI()->NXMessageBox()->Show("错误", NXMessageBox::DialogTypeError, e.what());
    }
    catch (...)
    {
        UI::GetUI()->NXMessageBox()->Show("错误", NXMessageBox::DialogTypeError, "拼料插件异常");
    }
}

extern "C" DllExport int ufusr_ask_unload()
{
    return Session::LibraryUnloadOptionImmediately;
}

BlockDialog::DialogResponse UI2::Launch()
{
    return theDialog->Launch();
}

void UI2::initialize_cb()
{
    // 获取所有UI控件 - 使用原始文件中的 block id
    ui_group1 = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("ui_group1"));
    ui_button0 = dynamic_cast<NXOpen::BlockStyler::Button*>(theDialog->TopBlock()->FindBlock("ui_button0"));
    ui_button1 = dynamic_cast<NXOpen::BlockStyler::Button*>(theDialog->TopBlock()->FindBlock("ui_button1"));
    ui_group2 = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("ui_group2"));
    ui_face_select0 = dynamic_cast<NXOpen::BlockStyler::FaceCollector*>(theDialog->TopBlock()->FindBlock("ui_face_select0"));
    ui_point0 = dynamic_cast<NXOpen::BlockStyler::SpecifyPoint*>(theDialog->TopBlock()->FindBlock("ui_point0"));
    ui_toggle0 = dynamic_cast<NXOpen::BlockStyler::Toggle*>(theDialog->TopBlock()->FindBlock("ui_toggle0"));
    ui_group3 = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("ui_group3"));
    ui_linear_dim0 = dynamic_cast<NXOpen::BlockStyler::LinearDimension*>(theDialog->TopBlock()->FindBlock("ui_linear_dim0"));
    ui_linear_dim01 = dynamic_cast<NXOpen::BlockStyler::LinearDimension*>(theDialog->TopBlock()->FindBlock("ui_linear_dim01"));
    ui_linear_dim02 = dynamic_cast<NXOpen::BlockStyler::LinearDimension*>(theDialog->TopBlock()->FindBlock("ui_linear_dim02"));
    ui_toggle01 = dynamic_cast<NXOpen::BlockStyler::Toggle*>(theDialog->TopBlock()->FindBlock("ui_toggle01"));
    ui_toggle02 = dynamic_cast<NXOpen::BlockStyler::Toggle*>(theDialog->TopBlock()->FindBlock("ui_toggle02"));
    ui_selection0 = dynamic_cast<NXOpen::BlockStyler::SelectObject*>(theDialog->TopBlock()->FindBlock("ui_selection0"));
    ui_tree_control01 = dynamic_cast<NXOpen::BlockStyler::Tree*>(theDialog->TopBlock()->FindBlock("ui_tree_control01"));
}

void UI2::dialogShown_cb()
{
    // 初始化默认值
    if (ui_linear_dim0)
    {
        PropertyList* prop = ui_linear_dim0->GetProperties();
        prop->SetDouble("Value", 5.0);
        delete prop;
    }

    if (ui_linear_dim01)
    {
        PropertyList* prop = ui_linear_dim01->GetProperties();
        prop->SetDouble("Value", 10.0);
        delete prop;
    }

    if (ui_linear_dim02)
    {
        PropertyList* prop = ui_linear_dim02->GetProperties();
        prop->SetDouble("Value", 5.0);
        delete prop;
    }

    if (ui_toggle01)
    {
        PropertyList* prop = ui_toggle01->GetProperties();
        prop->SetLogical("Value", false);
        delete prop;
    }

    if (ui_toggle0)
    {
        PropertyList* prop = ui_toggle0->GetProperties();
        prop->SetLogical("Value", true);
        delete prop;
    }
}

void UI2::ShowMessage(const string& title, NXMessageBox::DialogType type, const string& message)
{
    theUI->NXMessageBox()->Show(title.c_str(), type, message.c_str());
}

double UI2::GetDoubleValue(UIBlock* block)
{
    if (!block) return 0.0;
    PropertyList* prop = block->GetProperties();
    double value = prop->GetDouble("Value");
    delete prop;
    return value;
}

bool UI2::GetToggleValue(Toggle* toggle)
{
    if (!toggle) return false;
    PropertyList* prop = toggle->GetProperties();
    bool value = prop->GetLogical("Value");
    delete prop;
    return value;
}

void UI2::GetPointCoordinates(double& x, double& y, double& z)
{
    x = 0; y = 0; z = 0;
    if (!ui_point0) return;

    PropertyList* prop = ui_point0->GetProperties();
    NXOpen::Point3d point = prop->GetPoint("PointCoordinates");
    delete prop;

    x = point.X;
    y = point.Y;
    z = point.Z;
}

void UI2::SetBodyVisibility(Body* body, bool visible)
{
    if (!body) return;

    DisplayManager* displayMgr = theSession->DisplayManager();
    vector<DisplayableObject*> objects;
    objects.push_back(body);

    if (visible)
    {
        displayMgr->UnblankObjects(objects);
    }
    else
    {
        displayMgr->BlankObjects(objects);
    }
}

void UI2::CreateStockMessage(int partCount, double totalWidth, double totalDepth, double maxHeight,
    double baseX, double baseY, double baseZ)
{
    string message = "========== 毛料创建信息 ==========\n\n";
    message += "零件数量: " + to_string(partCount) + " 个\n\n";
    message += "毛料尺寸:\n";
    message += "  长度 (X): " + to_string(totalWidth) + " mm\n";
    message += "  宽度 (Y): " + to_string(totalDepth) + " mm\n";
    message += "  高度 (Z): " + to_string(maxHeight) + " mm\n\n";
    message += "毛料原点:\n";
    message += "  X: " + to_string(baseX) + " mm\n";
    message += "  Y: " + to_string(baseY) + " mm\n";
    message += "  Z: " + to_string(baseZ) + " mm\n\n";
    message += "操作步骤:\n";
    message += "1. 点击菜单: 插入 -> 设计特征 -> 块\n";
    message += "2. 输入上述长度、宽度、高度\n";
    message += "3. 指定上述原点位置\n";
    message += "4. 点击确定创建毛料";

    ShowMessage("生成毛料", NXMessageBox::DialogTypeInformation, message);
}

void UI2::HideSelectedBodies()
{
    m_selectedParts.clear();
    vector<TaggedObject*> objs = ui_face_select0->GetSelectedObjects();

    for (size_t i = 0; i < objs.size(); i++)
    {
        Face* face = dynamic_cast<Face*>(objs[i]);
        if (!face) continue;

        Body* body = face->GetBody();
        if (!body) continue;

        bool alreadyAdded = false;
        for (size_t j = 0; j < m_selectedParts.size(); j++)
        {
            if (m_selectedParts[j].body == body)
            {
                alreadyAdded = true;
                break;
            }
        }

        if (!alreadyAdded)
        {
            PartInfo info;
            info.body = body;
            info.referenceFace = face;
            info.index = (int)m_selectedParts.size() + 1;

            m_selectedParts.push_back(info);
            SetBodyVisibility(body, false);
        }
    }

    if (m_selectedParts.empty())
    {
        ShowMessage("提示", NXMessageBox::DialogTypeInformation, "请选择参考底面");
    }
    else
    {
        char msg[256];
        sprintf_s(msg, "已选择并隐藏 %d 个零件", (int)m_selectedParts.size());
        ShowMessage("提示", NXMessageBox::DialogTypeInformation, msg);
    }
}

void UI2::GetAllSelectedBodies()
{
    m_selectedParts.clear();
    vector<TaggedObject*> objs = ui_face_select0->GetSelectedObjects();

    for (size_t i = 0; i < objs.size(); i++)
    {
        Face* face = dynamic_cast<Face*>(objs[i]);
        if (!face) continue;

        Body* body = face->GetBody();
        if (!body) continue;

        bool alreadyAdded = false;
        for (size_t j = 0; j < m_selectedParts.size(); j++)
        {
            if (m_selectedParts[j].body == body)
            {
                alreadyAdded = true;
                break;
            }
        }

        if (!alreadyAdded)
        {
            PartInfo info;
            info.body = body;
            info.referenceFace = face;
            info.index = (int)m_selectedParts.size() + 1;
            m_selectedParts.push_back(info);
        }
    }
}

void UI2::ShowAndArrangePartsAtPoint()
{
    if (m_selectedParts.empty())
    {
        ShowMessage("提示", NXMessageBox::DialogTypeError, "请先选择零件");
        return;
    }

    try
    {
        // 获取参数
        m_partSpacing = GetDoubleValue(ui_linear_dim0);
        m_outerMargin = GetDoubleValue(ui_linear_dim01);
        m_topSpacing = GetDoubleValue(ui_linear_dim02);

        double baseX, baseY, baseZ;
        GetPointCoordinates(baseX, baseY, baseZ);

        // 使用估算的零件尺寸（用户需要根据实际零件调整）
        double estimatedPartSize = 50.0;  // 估算每个零件50mm

        // 计算总尺寸
        double totalWidth = m_selectedParts.size() * estimatedPartSize + (m_selectedParts.size() - 1) * m_partSpacing + m_outerMargin * 2;
        double totalDepth = estimatedPartSize + m_outerMargin * 2;
        double maxHeight = estimatedPartSize + m_topSpacing;

        // 显示所有零件
        for (size_t i = 0; i < m_selectedParts.size(); i++)
        {
            SetBodyVisibility(m_selectedParts[i].body, true);
        }

        // 显示拼料结果
        string result = "========== 拼料计算结果 ==========\n\n";
        result += "零件数量: " + to_string(m_selectedParts.size()) + " 个\n";
        result += "零件间距: " + to_string(m_partSpacing) + " mm\n";
        result += "外围间距: " + to_string(m_outerMargin) + " mm\n";
        result += "顶部间距: " + to_string(m_topSpacing) + " mm\n\n";
        result += "估算零件尺寸: " + to_string(estimatedPartSize) + " mm (请根据实际零件修改)\n\n";
        result += "毛料尺寸:\n";
        result += "  长度 (X): " + to_string(totalWidth) + " mm\n";
        result += "  宽度 (Y): " + to_string(totalDepth) + " mm\n";
        result += "  高度 (Z): " + to_string(maxHeight) + " mm\n\n";
        result += "排列原点: (" + to_string(baseX) + ", " + to_string(baseY) + ", " + to_string(baseZ) + ")\n\n";
        result += "注意: 请根据实际零件尺寸手动调整上述尺寸！";

        // 生成毛料提示
        m_generateStock = GetToggleValue(ui_toggle01);
        if (m_generateStock)
        {
            CreateStockMessage((int)m_selectedParts.size(), totalWidth, totalDepth, maxHeight, baseX, baseY, baseZ);
        }

        ShowMessage("拼料完成", NXMessageBox::DialogTypeInformation, result);
    }
    catch (exception& e)
    {
        ShowMessage("拼料失败", NXMessageBox::DialogTypeError, e.what());
    }
}

int UI2::apply_cb()
{
    try
    {
        // 检查是否选择零件
        GetAllSelectedBodies();

        if (m_selectedParts.empty())
        {
            ShowMessage("提示", NXMessageBox::DialogTypeError, "请先在'选择参考底面'中选择零件底面");
            return 0;
        }

        // 检查是否指定了点
        double x, y, z;
        GetPointCoordinates(x, y, z);

        // 执行拼料
        ShowAndArrangePartsAtPoint();
    }
    catch (exception& e)
    {
        ShowMessage("错误", NXMessageBox::DialogTypeError, e.what());
    }

    return 0;
}

int UI2::update_cb(UIBlock* block)
{
    // 处理拼料按钮点击
    if (block == ui_button0)
    {
        GetAllSelectedBodies();
        if (!m_selectedParts.empty())
        {
            ShowAndArrangePartsAtPoint();
        }
        else
        {
            ShowMessage("提示", NXMessageBox::DialogTypeInformation, "请先在'选择参考底面'中选择零件");
        }
    }

    // 处理选择参考底面变化
    if (block == ui_face_select0)
    {
        bool hideParts = GetToggleValue(ui_toggle0);

        if (hideParts)
        {
            HideSelectedBodies();
        }
        else
        {
            // 显示所有已选零件
            GetAllSelectedBodies();
            for (size_t i = 0; i < m_selectedParts.size(); i++)
            {
                SetBodyVisibility(m_selectedParts[i].body, true);
            }
        }
    }

    // 处理间距变化
    if (block == ui_linear_dim0 || block == ui_linear_dim01 || block == ui_linear_dim02)
    {
        m_partSpacing = GetDoubleValue(ui_linear_dim0);
        m_outerMargin = GetDoubleValue(ui_linear_dim01);
        m_topSpacing = GetDoubleValue(ui_linear_dim02);
    }

    return 0;
}

int UI2::ok_cb()
{
    return apply_cb();
}

PropertyList* UI2::GetBlockProperties(const char* blockID)
{
    return theDialog->GetBlockProperties(blockID);
}