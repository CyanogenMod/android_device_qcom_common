===============================================================================
Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
===============================================================================

1. Introduction:
----------------
    Android provides support for AppOps service to manage and control
    user-permissions of different operations of installed packages.

    AppOps service reads /system/etc/appops_policy.xml file to read all default
    user-permissions of different operations of installed packages.

2. AppOps Policy File:
----------------------
    AppOps policy file can be used to assign default permissions to both system
    and user installed applications. Since hardcoded in AppOpsService this file
    should be at /system/etc/appops_policy.xml path on the device.

    Following section talks about various tags used in appops_policy.xml file:

    2.1 appops-policy
    This is the root tag of appops_policy file.

        Parameters:
        i) version (Required) -This parameter defines the version of
            appops_policy.xml format. It should be set to "1".

        Tags:
        i) user-app - section to configure default permissions of user
                      installed apps.
        ii) system-app - section to configure default permissions of preset
                         (system) apps
        iii) application - section to override default permission of particular
                           packages.
    2.2 user-app
    This tag is used to set default permission of user installed applications.

    Parameters:
    i) permission - To set default permission of all user installed apps.
    ii) show - To show/hide operations in app ops settings activity.

    Tags:
    <None>

    2.3 system-app
    This tag is used to set default permission of preset installed applications.

    Parameters:
    i) permission - To set default permission of all system installed apps.
    ii) show - To show/hide operations in app ops settings activity.

    Tags:
    <None>

    2.4 application
    This tag is used to set default permission of specific applications.

    Parameters:
    <None>

    Tags:
    i) pkg - section to set default permission of specific package.

    2.5 pkg
    This tag is used to set default permission of specific package.

    Parameters:
    i) name - To specify name of the package.
    ii) type - To set if package is part of preset or user installed application.
    iii) permission - To set default permission of package's all operations.
    iv) show - To show/hide operations in app ops settings activity.

    Tags:
    i) op - section to set defualt permission of an operation.

    2.5 op
    This tag is used to set default permission of specific operation

    Parameters:
    i) name - To specify name of the operation
    ii) permission - To set default permisison of operation
    iii) show - To show/hide operation in app ops settings activity.

3 Example:
----------
<appops-policy version="1">
    <user-app permission="ask" show="true"/>
    <system-app permission="allowed" show="false"/>

    <application>
        <pkg name="com.android.dialer" type="system-app">
            <op name="android:call_phone" permission="ask" show="true"/>
        </pkg>
        <pkg name="com.xyz.abc" type="system-app" permission="allowed" show="false">
            <op name="android:fine_location" permission="ask" show="true"/>
            <op name="android:monitor_location" permission="ask" show="true"/>
        </pkg>
    </application>
</appops-policy>

