# CQ 码

CQ 码的使用方式和酷 Q 原生相同，在要发送的消息中插入相应的代码即可，例如：

```
[CQ:face,id=14] [CQ:image,file=1.jpg]
```

使用方式和原生 CQ 码（关于原生 CQ 码的使用，请看 [Pro/CQ码](https://d.cqp.me/Pro/CQ%E7%A0%81)）完全相同**意味着需要在某些情况下对字符进行转义**，由于很多时候我们不需要使用 CQ 码，只需要发送文字消息就行了，这种情况下可以在请求 API 时加入 `is_raw` 参数，这将会自动对整个消息的特殊字符进行转义，具体请看 [API 描述](https://richardchien.github.io/coolq-http-api/#/API)。

除了原生的 CQ 码，CoolQ HTTP API 还提供了一些实用的增强功能，后面称之为「增强 CQ 码」。

## 增强功能列表

### 发送网络图片或语音

酷 Q 原生的 CQ 码只能发送 `data\image` 文件夹里的图片、`data\record` 里的语音，增强 CQ 码支持设置 `file` 为网络链接，内部会首先把图片或语音下载到 `data` 中相应的文件夹，然后把 `file` 替换成下载好的本地文件名。例如：

```
[CQ:image,file=http://i1.piimg.com/567571/fdd6e7b6d93f1ef0.jpg]
[CQ:record,file=http://doora.qiniudn.com/35aIm.silk]
```

### 发送文件系统中另一个地方的图片或语音

除了发送网络上的图片、语音，还可以发送本地文件系统中其它地方的图片、语音，使用 `file://` 加文件的绝对路径，例如：

```
[CQ:image,file=file://C:\Users\richard\Pictures\1.png]
[CQ:record,file=file://C:\Users\richard\Music\1.mp3]
```

### 提取 cqimg 文件中的实际图片 URL

酷 Q 收到的图片会放在 `data\image` 中，并且以文件名加 `.cqimg` 扩展名的形式存储为纯文本文件，实际的图片 URL 就在里面的 `url` 字段，增强 CQ 码会自动提取这个 URL，并添加到 CQ 码的 `url` 参数中。

例如，假设原 CQ 码如下：

```
[CQ:image,file=AE3062186A2073B33AB1F2BB2F58F3A4.jpg]
```

提取 URL 后，会更改为：

```
[CQ:image,file=AE3062186A2073B33AB1F2BB2F58F3A4.jpg,url=http://183.232.95.26/offpic_new/1002647525//8102132e-4ab0-46cf-a8e1-2f62185232cb/0]
```

如果提取不成功（读取文件失败），则不变。

**另外请注意，这个 URL 会在一定时间后过期（不确定多久），但酷 Q 在收到以前收过的图之后，仍然会返回同样的文件名，因此建议定期清理 `data\image` 目录以防止 URL 失效。**

## 原生 CQ 码的非官方补充

下面给出一些在酷 Q 官方文档之外的补充，或者说坑的预告：

- `emoji` 码可能出现在昵称、群名片、群名称等各种其它酷 Q 产出的字符串中
- `face` 码接收时 id 在 0~170，为旧版 QQ 表情的 id，发送时可以发送 170 之后的新版 QQ 表情
- `bface` 码在酷 Q Air 中不给出 id，`data` 目录也不会收到表情缓存，但会在 CQ 码之后紧跟着给出 `&#91;表情标题&#93;`，而 Pro 中则正常
- 发送 `bface` 不止要指定 `id`，还要指定 `p`，即 `data\bface` 中的子目录名
- `sface` 码对应手机 QQ「表情商城」中的「小表情」
- 酷 Q Air 无法收到 `image` 码（直接不给出这个 CQ 码，同一消息的其它文本正常给出）
- 除了官方文档中给出的 CQ 码，在接收消息中还会出现 `contact` 联系人分享、`location` 位置分享、`show` 厘米秀、`sign` 签到、`rich` 其它富媒体分享
- 在 iOS 设备上的有限的测试指出，QQ 音乐分享到 QQ，不显示成 CQ 码，而是只有文本链接；网易云音乐分享的，CQ 码是 `rich`；全民 K 歌分享的也是 `rich`
- 同样在 iOS 设备上的测试，支持分享到 QQ 的其它（非音乐）app，分享的内容的 CQ 码均为 `rich`
- 收到文件将会给出纯文本的文件名和文件大小，而不是 CQ 码
- `shake` 和其它内容混发时，会被单独发出，去掉它的其它内容会合并在另一条中发送
- `record` 和其它内容混发时，只有它会被发出，其它内容会被忽略
- `rps`、`dice` 和其它内容混发时，其前面、本身、后面会依次分成三条消息发送；另外，它们在 TIM 上无法显示
- 在私聊或讨论组中使用 `anonymous` 时，无论是否给出 `ignore` 参数，消息都会被发出
- 6 月 9 日的测试显示目前无法在群聊中发送 `anonymous`
- 私聊和群聊中可以发送 `show`，`id` 参数可从获取的消息中取得，`qq` 参数不是必要的；讨论组无法发送
- `sign`、`rich` 码无法主动发送
