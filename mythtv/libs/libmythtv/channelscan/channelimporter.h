// -*- Mode: c++ -*-
/*
 *  Copyright (C) Daniel Kristjansson 2007
 *
 *  This file is licensed under GPL v2 or (at your option) any later version.
 *
 */

#ifndef _CHANNEL_IMPORTER_H_
#define _CHANNEL_IMPORTER_H_

// C++ headers
#include <cstring>

// Qt headers
#include <QMap>
#include <QString>
#include <QCoreApplication>

// MythTV headers
#include "mythtvexp.h"
#include "scaninfo.h"
#include "channelscantypes.h"
#include "mythmainwindow.h"

enum OkCancelType {
    kOCTCancelAll = -1,
    kOCTCancel    = +0,
    kOCTOk        = +1,
    kOCTOkAll     = +2,
};

class ChannelImporterBasicStats
{
  public:
    ChannelImporterBasicStats()
    {
        memset(m_atscChannels, 0, sizeof(m_atscChannels));
        memset(m_dvbChannels,  0, sizeof(m_dvbChannels));
        memset(m_scteChannels, 0, sizeof(m_scteChannels));
        memset(m_mpegChannels, 0, sizeof(m_mpegChannels));
        memset(m_ntscChannels, 0, sizeof(m_ntscChannels));
    }

    // totals
    uint m_atscChannels[3];
    uint m_dvbChannels [3];
    uint m_scteChannels[3];
    uint m_mpegChannels[3];
    uint m_ntscChannels[3];

    // per channel counts
    QMap<uint,uint>    m_progNumCnt;
    QMap<uint,uint>    m_atscNumCnt;
    QMap<uint,uint>    m_atscMinCnt;
    QMap<uint,uint>    m_atscMajCnt;
    QMap<QString,uint> m_chanNumCnt;
};

class ChannelImporterUniquenessStats
{
  public:
    ChannelImporterUniquenessStats() = default;

    uint m_uniqueProgNum {0};
    uint m_uniqueAtscNum {0};
    uint m_uniqueAtscMin {0};
    uint m_uniqueChanNum {0};
    uint m_uniqueTotal   {0};
    uint m_maxAtscMajCnt {0};
};

class MTV_PUBLIC ChannelImporter
{
    Q_DECLARE_TR_FUNCTIONS(ChannelImporter)

  public:
    ChannelImporter(bool gui, bool interactive,
                    bool _delete, bool insert, bool save,
                    bool fta_only, bool lcn_only, bool complete_only,
                    bool full_channel_search,
                    ServiceRequirements service_requirements,
                    bool success = false) :
        m_useGui(gui),
        m_isInteractive(interactive),
        m_doDelete(_delete),
        m_doInsert(insert),
        m_doSave(save),
        m_ftaOnly(fta_only),
        m_lcnOnly(lcn_only),
        m_completeOnly(complete_only),
        m_fullChannelSearch(full_channel_search),
        m_success(success),
        m_serviceRequirements(service_requirements) { }

    void Process(const ScanDTVTransportList&, int sourceid = -1);

  protected:
    enum DeleteAction
    {
        kDeleteAll,
        kDeleteManual,
        kDeleteIgnoreAll,
        kDeleteInvisibleAll,
    };
    enum InsertAction
    {
        kInsertAll,
        kInsertManual,
        kInsertIgnoreAll,
    };
    enum UpdateAction
    {
        kUpdateAll,
        kUpdateManual,
        kUpdateIgnoreAll,
    };

    enum ChannelType
    {
        kChannelTypeFirst = 0,

        kChannelTypeNonConflictingFirst = kChannelTypeFirst,
        kATSCNonConflicting = kChannelTypeFirst,
        kDVBNonConflicting,
        kSCTENonConflicting,
        kMPEGNonConflicting,
        kNTSCNonConflicting,
        kChannelTypeNonConflictingLast = kNTSCNonConflicting,

        kChannelTypeConflictingFirst,
        kATSCConflicting = kChannelTypeConflictingFirst,
        kDVBConflicting,
        kSCTEConflicting,
        kMPEGConflicting,
        kNTSCConflicting,
        kChannelTypeConflictingLast = kNTSCConflicting,
        kChannelTypeLast = kChannelTypeConflictingLast,
    };

    static QString toString(ChannelType type);

    static void CleanupDuplicates(ScanDTVTransportList &transports);
    void FilterServices(ScanDTVTransportList &transports) const;
    ScanDTVTransportList GetDBTransports(
        uint sourceid, ScanDTVTransportList&) const;

    uint DeleteChannels(ScanDTVTransportList&);
    uint DeleteUnusedTransports(uint sourceid);

    void InsertChannels(const ScanDTVTransportList&,
                        const ChannelImporterBasicStats&);

    ScanDTVTransportList InsertChannels(
        const ScanDTVTransportList &transports,
        const ChannelImporterBasicStats &info,
        InsertAction action,
        ChannelType type,
        ScanDTVTransportList &inserted,
        ScanDTVTransportList &skipped);

    ScanDTVTransportList UpdateChannels(
        const ScanDTVTransportList &transports,
        const ChannelImporterBasicStats &info,
        UpdateAction action,
        ChannelType type,
        ScanDTVTransportList &updated,
        ScanDTVTransportList &skipped);

    /// For multiple channels
    DeleteAction QueryUserDelete(const QString &msg);

    /// For multiple channels
    InsertAction QueryUserInsert(const QString &msg);

    /// For multiple channels
    UpdateAction QueryUserUpdate(const QString &msg);

    /// For a single channel
    OkCancelType QueryUserResolve(
        const ScanDTVTransport          &transport,
        ChannelInsertInfo               &chan);

    /// For a single channel
    OkCancelType QueryUserInsert(
        const ScanDTVTransport          &transport,
        ChannelInsertInfo               &chan);

    static QString ComputeSuggestedChannelNum(
        const ChannelInsertInfo         &chan);

    OkCancelType ShowManualChannelPopup(
        MythMainWindow *parent, const QString& title,
        const QString& message, QString &text);

    OkCancelType ShowResolveChannelPopup(
        MythMainWindow *parent, const QString& title,
        const QString& message, QString &text);

    static void FixUpOpenCable(ScanDTVTransportList &transports);

    static ChannelImporterBasicStats CollectStats(
        const ScanDTVTransportList &transports);

    static ChannelImporterUniquenessStats CollectUniquenessStats(
        const ScanDTVTransportList &transports,
        const ChannelImporterBasicStats &info);

    static QString FormatChannels(
        const ScanDTVTransportList      &transports,
        const ChannelImporterBasicStats *info = nullptr);

    static QString FormatChannel(
        const ScanDTVTransport          &transport,
        const ChannelInsertInfo         &chan,
        const ChannelImporterBasicStats *info = nullptr);

    static QString SimpleFormatChannel(
        const ScanDTVTransport          &transport,
        const ChannelInsertInfo         &chan);

    static QString FormatTransport(
        const ScanDTVTransport          &transport);

    static QString FormatTransports(
        const ScanDTVTransportList      &transports_in);

    static QString GetSummary(
        uint                                  transport_count,
        const ChannelImporterBasicStats      &info,
        const ChannelImporterUniquenessStats &stats);

    static bool IsType(
        const ChannelImporterBasicStats &info,
        const ChannelInsertInfo &chan, ChannelType type);

    static void CountChannels(
        const ScanDTVTransportList &transports,
        const ChannelImporterBasicStats &info,
        ChannelType type, uint &new_chan, uint &old_chan);

    static int SimpleCountChannels(
        const ScanDTVTransportList &transports);

    static bool CheckChannelNumber(
        const QString           &num,
        const ChannelInsertInfo &chan);

    static void AddChanToCopy(
        ScanDTVTransport &transport_copy,
        const ScanDTVTransport &transport,
        const ChannelInsertInfo &chan);

  private:
    bool                m_useGui;
    bool                m_isInteractive;
    bool                m_doDelete;
    bool                m_doInsert;
    bool                m_doSave;
    /// Only FreeToAir (non-encrypted) channels desired post scan?
    bool                m_ftaOnly;
    /// Only services with logical channel numbers desired post scan?
    bool                m_lcnOnly;
    /// Only services with complete scandata desired post scan?
    bool                m_completeOnly;
    /// Keep existing channel numbers on channel update
    bool                m_keepChannelNumbers      {true};
    /// Full search for old channels
    bool                m_fullChannelSearch       {false};
    /// To pass information IPTV channel scan succeeded
    bool                m_success {false};
    /// Services desired post scan
    ServiceRequirements m_serviceRequirements;

    QEventLoop          m_eventLoop;
};

#endif // _CHANNEL_IMPORTER_H_
