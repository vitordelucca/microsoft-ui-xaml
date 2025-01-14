﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using Common;
using MUXControlsTestApp.Utilities;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using System.Linq;
using System.Collections.Generic;

#if USING_TAEF
using WEX.TestExecution;
using WEX.TestExecution.Markup;
using WEX.Logging.Interop;
#else
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;
#endif

namespace Windows.UI.Xaml.Tests.MUXControls.ApiTests
{
    [TestClass]
    public class AutoSuggestBoxTests
    {
        [TestCleanup]
        public void TestCleanup()
        {
            TestUtilities.ClearVisualTreeRoot();
        }

        [TestMethod]
        public void VerifyAutoSuggestBoxCornerRadius()
        {
            if (PlatformConfiguration.IsOSVersionLessThan(OSVersion.Redstone5))
            {
                Log.Warning("AutoSuggestBox CornerRadius property is not available pre-rs5");
                return;
            }

            AutoSuggestBox autoSuggestBox = null;
            RunOnUIThread.Execute(() =>
            {
                autoSuggestBox = new AutoSuggestBox();
                List<string> suggestions = new List<string> 
                {
                    "Item 1", "Item 2", "Item 3"
                };
                autoSuggestBox.ItemsSource = suggestions;
            });
            IdleSynchronizer.Wait();
            Verify.IsNotNull(autoSuggestBox);
            TestUtilities.SetAsVisualTreeRoot(autoSuggestBox);

            RunOnUIThread.Execute(() =>
            {
                autoSuggestBox.CornerRadius = new CornerRadius(2);
                autoSuggestBox.Focus(FocusState.Keyboard);
                autoSuggestBox.Text = "123";
            });
            IdleSynchronizer.Wait();

            RunOnUIThread.Execute(() =>
            {
                var textBox = TestUtilities.FindDescendents<TextBox>(autoSuggestBox).Where(e => e.Name == "TextBox").Single();
                Verify.AreEqual(new CornerRadius(2, 2, 0, 0), textBox.CornerRadius);

                var popup = VisualTreeHelper.GetOpenPopups(Window.Current).Last();
                var popupBorder = popup.Child as Border;
                Verify.AreEqual(new CornerRadius(0, 0, 2, 2), popupBorder.CornerRadius);
            });
        }

    }
}
