/*
* CoolQ Demo for VC++
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "stdafx.h"
#include "appmain.h" // Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����

#include <sstream>
#include <curl/curl.h>
#include <jansson.h>
#include <event2/event.h>
#include <event2/http.h>
#include <WinSock2.h>

#include "encoding.h"
#include "misc_functions.h"
#include "request.h"

using namespace std;

int ac = -1; // AuthCode�����ÿ�Q�ķ���ʱ��Ҫ�õ�
bool enabled = false;

HANDLE httpd_thread_handle = NULL;
struct event_base *httpd_event_base = NULL;
struct evhttp *httpd_event = NULL;

/*
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char *, AppInfo, 0)
()
{
    return CQAPPINFO;
}

/*
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)
(int32_t AuthCode)
{
    ac = AuthCode;
    return 0;
}

/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ����������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)
()
{
    return 0;
}

/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)
()
{
    return 0;
}

DWORD WINAPI httpd_thread_func(LPVOID lpParam)
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    httpd_event_base = event_base_new();
    httpd_event = evhttp_new(httpd_event_base);

    evhttp_set_gencb(httpd_event, cqhttp_main_handler, NULL);
    evhttp_bind_socket(httpd_event, "0.0.0.0", 5700);
    LOG_D("HTTP�߳�", "��ʼ���� 0.0.0.0:5700\n");

    event_base_dispatch(httpd_event_base);
    return 0;
}

void start_httpd()
{
    httpd_thread_handle = CreateThread(NULL,              // default security attributes
                                       0,                 // use default stack size
                                       httpd_thread_func, // thread function name
                                       NULL,              // argument to thread function
                                       0,                 // use default creation flags
                                       NULL);             // returns the thread identifier
    if (!httpd_thread_handle)
    {
        LOG_E("����", "���� HTTP �ػ��߳�ʧ��");
    }
    else
    {
        LOG_D("����", "���� HTTP �ػ��̳߳ɹ�");
    }
}

void stop_httpd()
{
    if (httpd_thread_handle)
    {
        event_base_loopbreak(httpd_event_base);
        if (httpd_event)
        {
            evhttp_free(httpd_event);
        }
        if (httpd_event_base)
        {
            event_base_free(httpd_event_base);
        }
        WSACleanup();
        CloseHandle(httpd_thread_handle);
        httpd_thread_handle = NULL;
        httpd_event_base = NULL;
        httpd_event = NULL;
        LOG_D("ͣ��", "�ѹرպ�̨ HTTP �ػ��߳�")
    }
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ����������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)
()
{
    enabled = true;
    start_httpd();
    return 0;
}

/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)
()
{
    enabled = false;
    stop_httpd();
    return 0;
}

/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)
(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font)
{

    //���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
    //������ظ���Ϣ������֮���Ӧ��/���������������� return EVENT_IGNORE - ���Ա�����Ϣ

    string result = "";

    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "http://news-at.zhihu.com/api/4/news/latest");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_stringstream_callback);

        stringstream resp_stream;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_stream);

        CURLcode res;
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            string json_string = resp_stream.str();
            LOG8_D("Net>Json", string("Got json string: ") + json_string);
            json_t *data = json_loads(json_string.c_str(), 0, NULL);
            if (data)
            {
                LOG_D("Net>Json", "Succeeded to parse json data");
                stringstream ss;
                const char *date = json_string_value(json_object_get(data, "date"));
                ss << "Date: " << date << "\n\n";
                json_t *stories_jarr = json_object_get(data, "stories");
                for (size_t i = 0; i < json_array_size(stories_jarr); i++)
                {
                    const char *title = json_string_value(json_object_get(json_array_get(stories_jarr, i), "title"));
                    ss << (i == 0 ? "" : "\n") << i << ". " << title;
                }
                json_decref(data);
                result = ss.str();
            }
            else
            {
                LOG_D("Net>Json", "Failed to load json string");
            }
        }
        else
        {
            LOG_D("Net", "Failed to get response");
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        LOG_D("Net", "Failed to init cURL");
    }

    if (result != "")
    {
        CQ_sendPrivateMsg(ac, fromQQ, utf8_to_gbk(result.c_str()).c_str());
        CQ_sendPrivateMsg(ac, fromQQ, msg);
    }

    return EVENT_IGNORE;
}

/*
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=4 ��������Ϣ
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)
(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=201 �����¼�-����������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)
(int32_t subType, int32_t sendTime, int64_t fromQQ)
{

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=301 ����-��������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)
(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag)
{

    //CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=302 ����-Ⱥ����
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)
(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag)
{

    //if (subType == 1) {
    //	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
    //} else if (subType == 2) {
    //	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
    //}

    return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
CQEVENT(int32_t, __menuA, 0)
()
{
    MessageBoxA(NULL, "����menuA�����������봰�ڣ����߽�������������", "", 0);
    return 0;
}

CQEVENT(int32_t, __menuB, 0)
()
{
    MessageBoxA(NULL, "����menuB�����������봰�ڣ����߽�������������", "", 0);
    return 0;
}