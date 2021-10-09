# EMicro


### 简介

    基于注解的php框架，支持自定义注解解析

### 环境

* Linux
* php 最小版本7.4

### 部署
* 进入 emicro 文件夹
* 执行
```
 phpize && ./configure --with-php-config=yourpath/php-config && make && make install
```
* php.ini 文件 新增
```
 extension=emicro.so
```
* nginx 配置
```
    location / {
        if (!-e $request_filename) {
            rewrite  ^(.*)$  /index.php?s=$1  last;
            break;
        }
    }

```

### 推荐项目目录
```
    project
        |--application
            |--annotation【注解解析目录】
                |--Cache.php
                |--...
            |--config【配置文件目录】
                |--system.php[返回数组格式]
                |--...
            |--controller【控制器目录】
                |--Index.php
                |--...
        |--public
            |--index.php
        |--vendor【composer 安装的扩展，自动生成】
        |--composer.json【composer 扩展配置文件，自动生成】
```

### 快速开始

    案例中 orm 集成的是think-orm（composer require topthink/think-orm）,实际情况请根据自己的喜好集成

* public/index.php

```
<?php

    use EMicro\Application;

    // composer 扩展自动加载
    require_once "../vendor/autoload.php";

    // 获取引用实例
    $application = Application::getInstance();

    // 设置注解解析目录，不设置默认为【application/annotation】
    // $application->annotationNamespace('');
    // 设置控制器目录，不设置默认为【application/controller】
    // $application->dispatcherNamespace('');

    // 集成think-orm , 配置数据库
    \think\facade\Db::setConfig(config('database'));

    // 开始运行
    $application->run();

```

* application/controller/Index.php

```
<?php


    namespace controller;

    use EMicro\Request;

    /**
    * @Route(index)【路由地址，不设置默认为类名】
    */
    class Index
    {

        /**
        * @Route(hello)【路由地址，不设置默认为方法名，当前访问地址为：http://domain/index/hello】
        */
        public function index(Request $request){
            return 'hello EMicro!';
        }

    }

```

    接下来就可以愉快的玩耍了...

### 文档

```

 EMicro\Application
    |-- public static function getInstance(){}
    |-- public function dispatcherNamespace($dispatcherDir){}
    |-- public function run(){}
    |-- public function getAppPath(){}

```

```

 EMicro\Dispatcher
    |-- public static function getInstance(){}
    |-- public function controller(){}
    |-- public function method(){}

```

```

 EMicro\Factory
    |-- public static function call($class,$method,$params){}

```

```

 EMicro\Request
    |-- public static function get($param = '', $default=''){}
    |-- public static function post($param = '', $default=''){}
    |-- public static function input($param = '', $default=''){}
    |-- public static function server($param = '', $default=''){}
    |-- public static function cookie($param = '', $default=''){}
    |-- public static function files($param = ''){}
    |-- public static function header($param = '', $default=''){}
    |-- public static function isPost(){}
    |-- public static function isGet(){}
    |-- public static function isPut(){}
    |-- public static function isDelete(){}
    |-- public static function isPatch(){}
    |-- public static function isHead(){}

```

```
    全局函数 function config($param = '',$default=''){}

    内置注解 @Route(value),应用与控制器类和方法，如果不设置则默认使用【控制器类名称/控制器方法名称】

```
* 自定义注解
```
    比如在控制器上新增注解@Auth(params),则需要在application/annotation 中新增Auth类，实现run 方法，run方法接收2个参数，一个是注解中的参数，一个方法的返回值（适用后置注解）

    前置注解【在方法执行前执行】
        @Auth(params) 或 @Auth(params)!before

    后置注解【在方法执行后执行】
        @Auth(params)!after

    自己实现的类，比如自己实现的service或logic 也可使用注解，只是调用方式由
```
```
    $service = new Service();
    $returnData = $service->method($param);
               
                |
                |
               \|/

    EMicro\Factory::call()
```
变成
```

```

```
```

    <?php


    namespace annotation;


    class Auth
    {

        public function run($params, $returnData){
            var_dump('auth annotation');
            var_dump($params);
            var_dump($returnData);
        }

    }
```


* `git clone git@code.aliyun.com:minyea/egdd-admin.git && cd egdd-admin`
* 如无特殊需求不许改动此`docker`配置文件
* `cp laradock/docker.env laradock/.env`
* 启动容器 `开发环境使用增加: elasticsearch 容器`
* `docker-compose up -d nginx php-fpm workspace workspace-crontab php-worker-request-oss-compress-status`
* 配置项目
* `cp .env.example .env`
* 进入`workspace`执行命令
* `docker-compose exec workspace bash`
* `chmod -R 777 storage bootstrap/cache`
* `php artisan migrate`
* `php artisan db:seed`

****
* 启动队列容器
```
docker-compose up -d php-worker
```

## 容器说明
* `workspace`: 工作容器
* `workspace-crontab` 内包含定时任务执行,
* `php-worker`: 内包含队列任务,队列容器
* `elasticsearch`: 本地开发使用全文索引容器
* `php-worker-request-oss-compress-status`: 请求 oss 视频转码状态容器

## 重构容器
`docker-compose build nginx php-fpm workspace`

## 构建代码提示
```bash
php artisan ide-helper:models "App\Models\" --write --reset


php artisan ide-helper:models --write --reset
```

## 事项
* 添加或修改superior配置文件后要重构容器再启动才能生效, 即docker-compose build php-worker && docker-compose up -d php-worker
* 测试包含数据库条数限制

## 代码修复
```shell script
# 代码格式检测
phpcs --standard=PSR12
# 代码格式修复
phpcbf --standard=PSR12
# 代码质量分析
phpstan analyse
```
