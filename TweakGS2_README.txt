
  *** TweakGS2 V0.4.0 beta2 ****

  SC-02Cの設定アプリです
  要root

　※注意事項
　　テスト版です、すべての機能が正常動作しているわけではありません。
    不具合が出たら教えてくれないと治りません。

  ■出来ること
　・DualBoot対応(起動ROMの選択)
　・バックアップの管理 ※テスト中
　・CPU/GPU Control (要対応カーネル) ※GPUのみテスト中
　・Vibratorの強さ設定 (要対応カーネル/AOSP系のみ)
　・着信時にVibratorを強くする (要対応カーネル/AOSP系のみ)
　・着信時にキーバックライトを点滅させる (要対応カーネル)
  ・CIFS 有効/無効切り替え (要対応カーネル)
  ・logcat 有効/無効切り替え (要対応カーネル)
  ・mDNie 有効/無効切り替え (要対応カーネル)
  ・CRTアニメーション切り替え (要対応service.odex)
  ・Dockモードエミュレーション (HW Emulationは要対応カーネル)
  ・起動音 有効/無効切り替え (要対応カーネル)
  ・カメラ音 有効/無効切り替え (要対応カーネル)
  ・I/O Scheduler設定値変更
  ・LowMemKiller設定値調整
  ・VirtualMemory設定値調整
  ・Network設定の最適化(Network Boost)
  ・LCDのガンマ調整 (要対応カーネル)
  ・LCDの色調整 (要対応カーネル)
  ・HW Volumeの調整 (要対応カーネル)
　・シャープネス調整機能 (pending)

  ■設定のポイント

  ・起動音/カメラ音 有効/無効切り替え
    注意1 : 設定値は再起動後に有効になります
    注意2 : ro.camera.sound.forcedがどこにも設定されていないこと
            audioflinger.bootsndがどこにも設定されていないこと
    注意3 : あくまでも音鳴動の強制を抑制するものですのでサウンドOFF時のみ音が消えます
  
  ・Low Memory Killer
    お好みで

  ・Vitual Memory
    おすすめ設定
    /proc/sys/vm/swappiness: 0
    /proc/sys/vm/vfs_cache_pressure: 50
    /proc/sys/vm/dirty_expire_centisecs: 3000
    /proc/sys/vm/dirty_writeback_centisecs: 500
    /proc/sys/vm/dirty_ratio: 22
    /proc/sys/vm/dirty_background_ratio: 4 

  ・I/O Scheduler
    お好みで

  ・gamma設定
    お好みで

  ・LCD設定
    あまり上げたり下げたりしても良い効果は得られません
    +-5程度が良いと思います。

  ・HW Volume設定
    おすすめ設定は以下
    AVOL HP = -6
    DVOL DAC MASTER = 4
    DVOL DAC ATT = 2
    DVOL DIR0 = -4
    DVOL DIR0 ATT = -4

    ポイントはDACを上げてHPを下げる感じで調整すると良い感じです。

  ・Dock設定
    HW Emulation : カーネルレベルでドロイド君を騙します
    SW Emulation : アプリレベルでドロイド君を騙します
    ※通常はSW Emulationでご利用ください


　■更新履歴

    V0.4.0b2  書き込み先のファイルが無い場合にFileNotFoundExceptionが発生する
              不具合を修正

    V0.4.0b1  DualBoot時の起動ROM選択対応
              着信時の強制終了修正
              SystemProperty選択時のフリーズ修正
              etc...

    V0.4.0a3  GPU Control追加
              内部SDCARDパス変更
              install zip対応したけど多分ダメ

    V0.4.0a2  アイコン変更
              権限取得まわりの変更
              ROMManagerの外部SD対応
              LowMemoryの表示が逆になっていたので修正

    V0.4.0a1  バイブレベル調整機能を追加
              着信時にバイブを強くするオプションを追加
              着信時にBLNでお知らせする機能を追加
              DeepSleepチェック機能廃止

    V0.3.1    local.propにキーが重複することがある不具合を修正

    V0.3.0    起動時の設定に失敗することがある不具合を修正
              シャープネス設定機能追加
              DeepSleep確認機能追加
              お勧め設定値を設定する機能を追加(About内)

    V0.2.10   USB接続時に不必要なroot権限取得を行わないように変更
              (SW Emulation選択時はroot権限が必要なため取得します)
              起動時に設定値によっては強制終了する不具合を修正

    V0.2.9    CIFS設定機能追加

    V0.2.8    logcat設定機能追加
              mDNie設定機能追加

    V0.2.7    カーネルが対応していない場合、起動時にすべての設定が反映されない不具合を修正

    V0.2.6    CRTアニメーション切り替えが出来なくなる不具合を修正

    V0.2.5    リセット時local.propに初期値を入れるように修正
              CRTアニメーション切り替え機能追加

    V0.2.4    Color adjustmentを選択すると強制終了する不具合を修正

    V0.2.3    色々とBugfix
              カーネルで対応されていない項目の無効化
              値調整ボタン追加

    V0.2.2    Dock設定追加

    V0.2.1    boot sound設定機能追加
              camera sound設定機能追加
              I/O Scheduler設定機能追加

    V0.2.0    LowMemKiller設定機能追加
              VirtualMemory設定機能追加
              設定リセット追加 -> About内
　　　　　　   SPのVolumeが保存されない不具合を修正

    V0.1.3    ガンマ調整設定可能値を-50～50に変更
　　　　　　   現在の設定値をsammaryに表示


