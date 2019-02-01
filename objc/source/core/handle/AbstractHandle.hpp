/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _WCDB_ABSTRACTHANDLE_HPP
#define _WCDB_ABSTRACTHANDLE_HPP

#include <WCDB/ColumnMeta.hpp>
#include <WCDB/ErrorProne.hpp>
#include <WCDB/HandleNotification.hpp>
#include <WCDB/HandleStatement.hpp>
#include <WCDB/String.hpp>
#include <WCDB/WINQ.h>
#include <set>
#include <vector>

namespace WCDB {

class AbstractHandle : public ErrorProne {
#pragma mark - Initialize
public:
    AbstractHandle();

    AbstractHandle(const AbstractHandle &) = delete;
    AbstractHandle &operator=(const AbstractHandle &) = delete;
    virtual ~AbstractHandle() = 0;

    // Developers can call sqlite interfaces those WCDB does not provided currently by using this raw handle.
    // Note that this is not tested, which means that it may result in an unpredictable behavior.
    // Usage:
    //  e.g. 1. sqlite3* rawHandle = getRawHandle()
    //  e.g. 3. sqlite3_exec(rawHandle, ...)
    sqlite3 *getRawHandle();

protected:
    friend class HandleRelated;
    sqlite3 *m_handle;

#pragma mark - Global
public:
    static void enableMultithread();
    static void enableMemoryStatus(bool enable);
    static void setMemoryMapSize(int64_t defaultSizeLimit, int64_t maximumAllowedSizeLimit);

    typedef void (*GlobalLog)(void *, int, const char *);
    static void
    setNotificationForGlobalLog(const GlobalLog &log, void *parameter = nullptr);

    typedef int (*VFSOpen)(const char *, int, int);
    static void setNotificationWhenVFSOpened(const VFSOpen &vfsOpen);

#pragma mark - Path
public:
    void setPath(const String &path);
    const String &getPath() const;

    static String getSHMSubfix();
    static String getWALSubfix();
    static String getJournalSubfix();

private:
    String m_path;

#pragma mark - Basic
public:
    bool open();
    void close();
    bool isOpened() const;

    long long getLastInsertedRowID();
    const char *getErrorMessage();
    int getExtendedErrorCode();
    int getResultCode();
    int getChanges();
    bool isReadonly();
    bool isInTransaction();

    int getNumberOfDirtyPages();

    void interrupt(); // It's thread safe.

    void disableCheckpointWhenClosing(bool disable);

protected:
    bool executeSQL(const String &sql);
    bool executeStatement(const Statement &statement);

#pragma mark - Statement
protected:
    HandleStatement *getStatement();
    void returnStatement(HandleStatement *handleStatement);

private:
    void finalizeStatements();
    std::list<HandleStatement> m_handleStatements;

#pragma mark - Meta
public:
    std::pair<bool, bool> ft3TokenizerExists(const String &tokenizer);

    std::pair<bool, bool> tableExists(const String &table);
    std::pair<bool, bool> tableExists(const Schema &schema, const String &table);

    std::pair<bool, std::set<String>>
    getColumns(const Schema &schema, const String &table);
    std::pair<bool, std::set<String>> getColumns(const String &table);

    std::pair<bool, std::vector<ColumnMeta>>
    getTableMeta(const Schema &schema, const String &table);

protected:
    std::pair<bool, std::set<String>> getValues(const Statement &statement, int index);

#pragma mark - Transaction
public:
    bool beginTransaction();
    bool commitOrRollbackTransaction();
    void rollbackTransaction();

    bool beginNestedTransaction();
    bool commitOrRollbackNestedTransaction();
    void rollbackNestedTransaction();

    void enableLazyNestedTransaction(bool enable);

private:
    static const String &savepointPrefix();
    int m_nestedLevel;
    bool m_lazyNestedTransaction;

#pragma mark - Cipher
public:
    void setCipherKey(const UnsafeData &data);

#pragma mark - Notification
public:
    typedef HandleNotification::PerformanceNotification PerformanceNotification;
    void setNotificationWhenPerformanceTraced(const String &name,
                                              const PerformanceNotification &onTraced);

    typedef HandleNotification::SQLNotification SQLNotification;
    void setNotificationWhenSQLTraced(const String &name, const SQLNotification &onTraced);

    typedef HandleNotification::CommittedNotification CommittedNotification;
    void setNotificationWhenCommitted(int order,
                                      const String &name,
                                      const CommittedNotification &onCommitted);

    typedef HandleNotification::CheckpointedNotification CheckpointedNotification;
    void setNotificationWhenCheckpointed(const String &name,
                                         const CheckpointedNotification &checkpointed);
    void unsetNotificationWhenCommitted(const String &name);

    typedef HandleNotification::BusyNotification BusyNotification;
    void setNotificationWhenBusy(const BusyNotification &busyNotification);

    typedef HandleNotification::StatementDidStepNotification StatementDidStepNotification;
    void setNotificationWhenStatementDidStep(const String &name,
                                             const StatementDidStepNotification &notification);

    typedef HandleNotification::StatementWillStepNotification StatementWillStepNotification;
    void setNotificationWhenStatementWillStep(const String &name,
                                              const StatementWillStepNotification &notification);

private:
    HandleNotification m_notification;

#pragma mark - Error
public:
    // call it as push/pop in stack structure.
    void markErrorAsIgnorable(int ignorableCode);
    void markErrorAsUnignorable();

private:
    // The level of error will be "Ignore" if it's marked as ignorable.
    // But the return value will be still false.
    bool exitAPI(int rc);
    bool exitAPI(int rc, const String &sql);
    bool exitAPI(int rc, const char *sql);

    static bool isError(int rc);
    void notifyError(int rc, const char *sql);

    std::vector<int> m_ignorableCodes;
};

} //namespace WCDB

#endif /* _WCDB_ABSTRACTHANDLE_HPP */
