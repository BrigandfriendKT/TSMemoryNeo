# TSMemoryNeo

■はじめに

　このTSMemoryNeoはTSMemoryの後継ソフトとなりますが、<br>
そのTSMemoryの初登場は2008年8月で今年で19年目を迎えました。<br>
今もこうして現存するのは作者様はじめ、このソフトに携わり支えていただいた方、<br>
DTV界隈の方々、関係者様のご支援があったからだと感じております。（私自身も随分お世話になりました。）<br>
この場をお借りしあらため感謝申し上げます。ありがとうございます。

## TSMemoryからの変更点
　・Aviutl２の対応（従来のAviutlは非対応）<br>
　・スカパープレミアム、４K/８K放送のキャプチャー<br>
　・解像度のダウンコンバート機能追加<br>
　・UI周りの作り直し<br>

## 同梱ファイル
　TSMemoryNeo.tvtp　/　TSMemoryNeo.ini　/　CaptureUtil.aux2　/　その他<br>
　　CaptureUtil.aux2だけでもキャプチャーは可能。<br>
　　TVtestと連携させたい場合はTSMemoryNeo(tvtp,ini）が必要。
  
## 導入準備
事前にAviutl２およびL-smashの導入をお願いします。<br>
TVtest利用されない方は手順（２）から

（１）tvtest（実行ファイル）があるディレクトリにPluginsフォルダに "TSMemoryNeo.tvtp" を入れる。<br>
　　　また、TSMemoryNeo.iniにAviutl２のパスを指定、memorysizeも任意で変更する。<br>
　　　　（例）C:\Users\manekikecak\Desktop\aviutl2\aviutl2.exe<br>
    
（２）Aviutl２を起動のうえ "CaptureUtil.aux2" をD&Dしインストールする。<br>

（３）表示タブからTSMemoryNeo Panelを選択する。<br>
　　　保存先、拡張子、圧縮レベルなどを設定のうえ保存ボタンをクリックすることで<br>
　　　Captureutil.iniがCaptureutil.aux2と同じディレクトリに作成される。（以後、こちらからでも変更可能）

## 使用方法（TVtestの場合）

（１）tvtestを起動後プラグインを有効、tvtest設定からキー割り当てにて "TSMemoryNeo:実行"に任意のキーを付与しOKを押す。<br>

（２）任意のキーを押すことでAviutl２が起動、キャプチャーしたいところで保存ボタンをクリック。<br>
　　　（任意で保存先、解像度、拡張子、圧縮レベル、インターレース解除を設定）<br>

## 使用方法（Aviutl２の場合)

（１）Aviutl２に動画ファイルを任意のLayerにD&Dする。<br>

（２）任意のフレームで保存ボタンをクリック。<br>
　　　（任意で保存先、解像度、拡張子、圧縮レベル、インターレース解除を設定）<br>

### ～番外編～

Aviutl２のLayerごとに動画素材を載せて保存することや<br>
フィルタオブジェクトを載せて保存することも可能です。<br><br>

動画の重ねた状態でのキャプチャー<br>
<img width="320" height="180" alt="image_029" src="https://github.com/user-attachments/assets/3d54d421-49e9-44d8-862e-bcfb8c897f4a" />

ぼかし効果　キャプチャー<br>
<img width="320" height="180" alt="image_027" src="https://github.com/user-attachments/assets/d4822472-8f85-4567-b802-bebee1c154b7" />



