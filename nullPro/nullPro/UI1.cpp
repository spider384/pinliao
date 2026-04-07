//==============================================================================
// 拼料功能 NXOpen C++ 实现 (NX2306 + VS2022)
//==============================================================================
#include "UI2.hpp"
#include <NXOpen/BodyCollection.hxx>
#include <NXOpen/DisplayManager.hxx>
#include <NXOpen/Part.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/PointCollection.hxx>
#include <NXOpen/Unit.hxx>
#include <NXOpen/Features_FeatureCollection.hxx>
#include <NXOpen/Features_BodyFeature.hxx>
#include <cmath>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace std;

Session* UI2::theSession = NULL;
UI* UI2::theUI = NULL;

vector<Face*> g_selectedFaces;
vector<Body*> g_selectedBodies;
Point3d g_insertPoint;
bool g_isHideEnabled = true;

UI2::UI2()
{
    try
    {
        UI2::theSession = Session::GetSession();
        UI2::theUI = UI::GetUI();
        theDlxFileName = "UI 2.dlx";
        theDialog = UI2::theUI->CreateDialog(theDlxFileName);

        theDialog->AddApplyHandler(make_callback(this, &UI2::apply_cb));
        theDialog->AddOkHandler(make_callback(this, &UI2::ok_cb));
        theDialog->AddUpdateHandler(make_callback(this, &UI2::update_cb));
        theDialog->AddInitializeHandler(make_callback(this, &UI2::initialize_cb));
        theDialog->AddDialogShownHandler(make_callback(this, &UI2::dialogShown_cb));
    }
    catch (exception& ex)
    {
        throw;
    }
}

UI2::~UI2()
{
    if (theDialog != NULL)
    {
        delete theDialog;
        theDialog = NULL;
    }
}

extern "C" DllExport void ufusr(char* param, int* retcod, int param_len)
{
    UI2* theUI2 = NULL;
    try
    {
        theUI2 = new UI2();
        theUI2->Launch();
    }
    catch (exception& ex)
    {
        UI2::theUI->NXMessageBox()->Show("拼料", NXMessageBox::DialogTypeError, ex.what());
    }
    if (theUI2 != NULL)
    {
        delete theUI2;
        theUI2 = NULL;
    }
}

extern "C" DllExport int ufusr_ask_unload()
{
    return (int)Session::LibraryUnloadOptionImmediately;
}

extern "C" DllExport void ufusr_cleanup(void)
{
}

BlockDialog::DialogResponse UI2::Launch()
{
    BlockDialog::DialogResponse res = BlockDialog::DialogResponseInvalid;
    try
    {
        res = theDialog->Launch();
    }
    catch (exception& ex)
    {
        theUI->NXMessageBox()->Show("错误", NXMessageBox::DialogTypeError, ex.what());
    }
    return res;
}

void UI2::initialize_cb()
{
    try
    {
        group1 = dynamic_cast<Group*>(theDialog->TopBlock()->FindBlock("group1"));
        button0 = dynamic_cast<Button*>(theDialog->TopBlock()->FindBlock("button0"));
        button01 = dynamic_cast<Button*>(theDialog->TopBlock()->FindBlock("button01"));
        group0 = dynamic_cast<Group*>(theDialog->TopBlock()->FindBlock("group0"));
        face_select0 = dynamic_cast<FaceCollector*>(theDialog->TopBlock()->FindBlock("face_select0"));
        point0 = dynamic_cast<SpecifyPoint*>(theDialog->TopBlock()->FindBlock("point0"));
        toggle0 = dynamic_cast<Toggle*>(theDialog->TopBlock()->FindBlock("toggle0"));
        group = dynamic_cast<Group*>(theDialog->TopBlock()->FindBlock("group"));
        linear_dim0 = dynamic_cast<LinearDimension*>(theDialog->TopBlock()->FindBlock("linear_dim0"));
        linear_dim01 = dynamic_cast<LinearDimension*>(theDialog->TopBlock()->FindBlock("linear_dim01"));
        linear_dim02 = dynamic_cast<LinearDimension*>(theDialog->TopBlock()->FindBlock("linear_dim02"));
        toggle01 = dynamic_cast<Toggle*>(theDialog->TopBlock()->FindBlock("toggle01"));
        toggle02 = dynamic_cast<Toggle*>(theDialog->TopBlock()->FindBlock("toggle02"));
        string0 = dynamic_cast<StringBlock*>(theDialog->TopBlock()->FindBlock("string0"));
        string01 = dynamic_cast<StringBlock*>(theDialog->TopBlock()->FindBlock("string01"));
        selection0 = dynamic_cast<SelectObject*>(theDialog->TopBlock()->FindBlock("selection0"));
        tree_control01 = dynamic_cast<Tree*>(theDialog->TopBlock()->FindBlock("tree_control01"));

        if (linear_dim0) linear_dim0->SetValue(5.0);
        if (linear_dim01) linear_dim01->SetValue(10.0);
        if (linear_dim02) linear_dim02->SetValue(10.0);
        if (toggle01) toggle01->SetValue(true);
        if (toggle0) toggle0->SetValue(true);
    }
    catch (exception& ex)
    {
        theUI->NXMessageBox()->Show("初始化", NXMessageBox::DialogTypeError, ex.what());
    }
}

void UI2::dialogShown_cb()
{
}

// 隐藏选中零件
void UI2::HideSelectedBodies()
{
    Part* workPart = theSession->Parts()->Work();
    if (!workPart) return;

    g_selectedBodies.clear();
    vector<Face*> faces = face_select0->GetSelectedFaces();
    g_selectedFaces = faces;

    for (Face* face : faces)
    {
        Body* body = dynamic_cast<Body*>(face->OwningBody());
        if (body)
        {
            g_selectedBodies.push_back(body);
            DisplayManager::SetObjectVisibility(body, false);
        }
    }
}

// 在指定点拼接零件
void UI2::InsertAndMergeParts()
{
    Part* workPart = theSession->Parts()->Work();
    if (!workPart || g_selectedBodies.empty()) return;

    g_insertPoint = point0->GetPoint();
    double spacing = linear_dim0->GetValue();
    int cnt = g_selectedBodies.size();
    int cols = (int)sqrt(cnt) + 1;
    vector<Body*> newBodies;

    for (int i = 0; i < cnt; i++)
    {
        Body* src = g_selectedBodies[i];
        if (!src) continue;

        int r = i / cols;
        int c = i % cols;
        double w = GetBodyWidth(src);
        double h = GetBodyHeight(src);

        Point3d pos(
            g_insertPoint.X + c * (w + spacing),
            g_insertPoint.Y + r * (h + spacing),
            g_insertPoint.Z
        );

        Body* copy = CopyBody(workPart, src);
        MoveBody(copy, pos);
        newBodies.push_back(copy);
    }

    if (newBodies.size() > 1)
        MergeBodies(workPart, newBodies);

    if (toggle01->GetValue())
        CreateStockBody(workPart, newBodies);
}

double UI2::GetBodyWidth(Body* b)
{
    if (!b) return 10;
    BoundingBox3d box = b->GetBoundingBox();
    return box.MaxX - box.MinX;
}

double UI2::GetBodyHeight(Body* b)
{
    if (!b) return 10;
    BoundingBox3d box = b->GetBoundingBox();
    return box.MaxY - box.MinY;
}

Body* UI2::CopyBody(Part* part, Body* src)
{
    if (!part || !src) return NULL;
    return part->Features()->CopyBody(src);
}

void UI2::MoveBody(Body* b, Point3d p)
{
    if (!b) return;
    Vector3d vec(p.X, p.Y, p.Z);
    Matrix3x3 mat; mat.Identity();
    b->Move(Point3d(0, 0, 0), vec, mat);
}

void UI2::MergeBodies(Part* part, vector<Body*>& bs)
{
    if (bs.size() < 2) return;
    for (size_t i = 1; i < bs.size(); i++)
        part->Features()->Unite(bs[0], bs[i]);
}

// 生成毛料
void UI2::CreateStockBody(Part* part, vector<Body*>& parts)
{
    if (parts.empty()) return;

    double outer = linear_dim01->GetValue();
    double top = linear_dim02->GetValue();

    BoundingBox3d box = parts[0]->GetBoundingBox();
    for (Body* b : parts)
    {
        BoundingBox3d bbox = b->GetBoundingBox();
        box.MinX = min(box.MinX, bbox.MinX);
        box.MinY = min(box.MinY, bbox.MinY);
        box.MinZ = min(box.MinZ, bbox.MinZ);
        box.MaxX = max(box.MaxX, bbox.MaxX);
        box.MaxY = max(box.MaxY, bbox.MaxY);
        box.MaxZ = max(box.MaxZ, bbox.MaxZ);
    }

    Point3d p1(box.MinX - outer, box.MinY - outer, box.MinZ - outer);
    Point3d p2(box.MaxX + outer, box.MaxY + outer, box.MaxZ + top);

    Point* pt1 = part->Points()->CreatePoint(p1);
    Point* pt2 = part->Points()->CreatePoint(p2);
    Body* stock = part->Bodies()->CreateBox(pt1, pt2);
    DisplayManager::SetColor(stock, 150);
}

int UI2::apply_cb()
{
    int err = 0;
    try
    {
        InsertAndMergeParts();
        face_select0->ClearSelectedFaces();
        g_selectedFaces.clear();
        g_selectedBodies.clear();
    }
    catch (exception& ex)
    {
        err = 1;
        theUI->NXMessageBox()->Show("拼料", NXMessageBox::DialogTypeError, ex.what());
    }
    return err;
}

int UI2::update_cb(UIBlock* block)
{
    try
    {
        if (block == button0)
        {
            theUI->NXMessageBox()->Show("提示", NXMessageBox::DialogTypeInformation,
                "选择参考面 → 指定点 → 应用");
        }
        else if (block == button01)
        {
            face_select0->ClearSelectedFaces();
            point0->SetPoint(Point3d(0, 0, 0));
            g_selectedFaces.clear();
            g_selectedBodies.clear();

            Part* part = theSession->Parts()->Work();
            if (part)
                for (Body* b : part->Bodies())
                    DisplayManager::SetObjectVisibility(b, true);
        }
        else if (block == face_select0)
        {
            if (toggle0->GetValue())
                HideSelectedBodies();
        }
        else if (block == toggle0)
        {
            g_isHideEnabled = toggle0->GetValue();
        }
    }
    catch (exception& ex)
    {
        theUI->NXMessageBox()->Show("更新", NXMessageBox::DialogTypeError, ex.what());
    }
    return 0;
}

int UI2::ok_cb()
{
    return apply_cb();
}

PropertyList* UI2::GetBlockProperties(const char* id)
{
    return theDialog->GetBlockProperties(id);
}