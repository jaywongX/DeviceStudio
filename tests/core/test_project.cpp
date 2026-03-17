/**
 * @file test_project.cpp
 * @brief 项目管理器单元测试
 * @author DeviceStudio Team
 * @date 2026-03-16
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include "core/project/project.h"

using namespace DeviceStudio;

class ProjectManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        projectManager_ = ProjectManager::instance();
        projectManager_->closeProject();
    }
    
    void TearDown() override
    {
        projectManager_->closeProject();
    }
    
    ProjectManager* projectManager_;
};

TEST_F(ProjectManagerTest, SingletonInstance)
{
    auto instance1 = ProjectManager::instance();
    auto instance2 = ProjectManager::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(ProjectManagerTest, CreateProject)
{
    QString tempPath = QDir::tempPath() + "/test_project.dsproj";
    
    // 清理可能存在的文件
    if (QFile::exists(tempPath)) {
        QFile::remove(tempPath);
    }
    
    ProjectConfig config;
    config.name = "Test Project";
    config.description = "Test Description";
    config.author = "Test Author";
    config.version = "1.0.0";
    
    bool result = projectManager_->createProject(tempPath, config);
    EXPECT_TRUE(result);
    EXPECT_TRUE(projectManager_->hasOpenProject());
    EXPECT_EQ(projectManager_->config().name, "Test Project");
    
    // 清理
    QFile::remove(tempPath);
}

TEST_F(ProjectManagerTest, OpenProject)
{
    QString tempPath = QDir::tempPath() + "/test_open_project.dsproj";
    
    // 先创建项目
    ProjectConfig config;
    config.name = "Open Test";
    projectManager_->createProject(tempPath, config);
    projectManager_->closeProject();
    
    // 然后打开
    bool result = projectManager_->openProject(tempPath);
    EXPECT_TRUE(result);
    EXPECT_TRUE(projectManager_->hasOpenProject());
    EXPECT_EQ(projectManager_->config().name, "Open Test");
    
    // 清理
    QFile::remove(tempPath);
}

TEST_F(ProjectManagerTest, SaveProject)
{
    QString tempPath = QDir::tempPath() + "/test_save_project.dsproj";
    
    ProjectConfig config;
    config.name = "Save Test";
    projectManager_->createProject(tempPath, config);
    
    // 修改配置
    config.description = "Modified";
    projectManager_->updateConfig(config);
    
    // 保存
    bool result = projectManager_->saveProject();
    EXPECT_TRUE(result);
    
    // 重新打开验证
    projectManager_->closeProject();
    projectManager_->openProject(tempPath);
    EXPECT_EQ(projectManager_->config().description, "Modified");
    
    // 清理
    QFile::remove(tempPath);
}

TEST_F(ProjectManagerTest, CloseProject)
{
    QString tempPath = QDir::tempPath() + "/test_close_project.dsproj";
    
    ProjectConfig config;
    config.name = "Close Test";
    projectManager_->createProject(tempPath, config);
    
    EXPECT_TRUE(projectManager_->hasOpenProject());
    
    projectManager_->closeProject();
    EXPECT_FALSE(projectManager_->hasOpenProject());
    
    // 清理
    QFile::remove(tempPath);
}

TEST_F(ProjectManagerTest, MarkModified)
{
    QString tempPath = QDir::tempPath() + "/test_modified_project.dsproj";
    
    ProjectConfig config;
    config.name = "Modified Test";
    projectManager_->createProject(tempPath, config);
    
    EXPECT_FALSE(projectManager_->isModified());
    
    projectManager_->markModified();
    EXPECT_TRUE(projectManager_->isModified());
    
    // 清理
    QFile::remove(tempPath);
}

TEST_F(ProjectManagerTest, RecentProjects)
{
    projectManager_->clearRecentProjects();
    
    QString tempPath = QDir::tempPath() + "/test_recent.dsproj";
    ProjectConfig config;
    config.name = "Recent Test";
    projectManager_->createProject(tempPath, config);
    
    QStringList recent = projectManager_->recentProjects();
    EXPECT_FALSE(recent.isEmpty());
    EXPECT_TRUE(recent.contains(tempPath));
    
    // 清理
    QFile::remove(tempPath);
}
