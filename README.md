# snownet
����skynet���Լ�ѧϰ����Ŀ
Ŀ�ģ�֧��windows��linux��freeBSD
������Ϊ��ѩ��

/*
�Ҷ�skynet��ܵ����
1,skynet-src
	a)�����������̺߳�N�������߳�
	b)ͨ��snlua��ϲ����𶯲�ͬ��lua�߼�����
2,service-src
	a)snlua�Ǻ��ķ��񣬼���lua��ִ��,���������lua,����python�Ľű���
		ģ���������дһ���µķ��񣬲��������е�bootstrap = "snlua bootstrap" �ĳ� bootstrap = "snpython bootstrap"
	b)�����������������Ͽ���ֱ����luaд
	c)���������������Ҫ���ܵĴ���Ҳ��������д
3,lualib-src
	ͨ��luaopen_�������luaL_Regע��"C���Ժ��Ľӿ�"����������Ŀ�ܽ������磺
	ע�����push or pop��Ϣ����
4,lualib
	����"C���Ժ��Ľӿ�"����װ��"lua���Ľӿ�"���ṩ�������߼��������
5,service
	luaд���߼��������"lua���Ľӿ�"����������������������"lua���Ľӿ�"���ã���ֻ��Ҫ��server��д�߼�����lua�͹���
	�����������Ҫ����lualib-src��дһ���������Ӷ�Ӧ�Ľӿڣ�Ȼ����lualib-src��װ�ӿڲ��ṩlua�ӿڣ�������߼�����������ü���
6,3rd
	a)jemalloc:��һ����˵��ò�����ڴ�������
	b)lpeg:��ʲô�õģ�������
	c)lua:�ƴ��д��luaԴ����
	d)lua-md5:��֪��ΪʲôҪд��md5���ѵ�lua����û��md5�Ĺ��ܣ�
7,test��examples
	ûʲô��д�ģ��ο��ĵط�
*/